#version 330 core
out vec4 FragColor;

in vec3 Normal;  
in vec3 FragPos;
in vec2 TexCoords;
  
uniform vec3 lightPos; 
uniform vec3 viewPos; 
uniform vec3 lightColor;
uniform vec3 objectColor;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_normal1;

uniform sampler2D shadowMap;
in vec4 FragPosLightSpace; // Trebuie calculat și transmis din vertex shader

//float ShadowCalculation(vec4 fragPosLightSpace) {
//    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w; // Convertire în [-1, 1]
//    projCoords = projCoords * 0.5 + 0.5; // Convertire în [0, 1]

//    if (projCoords.z > 1.0)
//        return 0.0;

//    float closestDepth = texture(shadowMap, projCoords.xy).r;
//    float currentDepth = projCoords.z;

//    float bias = 0.005; // Ajustează bias-ul pentru artefacte
//    return currentDepth > closestDepth + bias ? 1.0 : 0.0;
//}

float ShadowCalculation(vec4 fragPosLightSpace) {
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;

    if (projCoords.z > 1.0)
        return 0.0;

    float shadow = 0.0;
    float bias = 0.005; // Bias ajustat
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0); // Dimensiunea unui texel

    // Filtrare 3x3 PCF
    for (int x = -1; x <= 1; ++x) {
        for (int y = -1; y <= 1; ++y) {
            float closestDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += (projCoords.z > closestDepth + bias) ? 1.0 : 0.0;
        }
    }
    shadow /= 9.0; // Media eșantioanelor

    return shadow;
}


void main()
{
    vec3 texColor = texture(texture_diffuse1, TexCoords).rgb;

    // ambient
    float ambientStrength = 0.5;
    vec3 ambient = ambientStrength * lightColor;

    // diffuse
    float diffuseStrength = 1.0;
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diffuseStrength * diff * lightColor;

    // specular
    float specularStrength = 1.0;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * lightColor;

    // Calculare umbră
    float shadow = ShadowCalculation(FragPosLightSpace);

    // Aplicare iluminare cu umbră
    vec3 result = (ambient + (1.0 - shadow) * (diffuse + specular)) * texColor;
    FragColor = vec4(result, 1.0);
}