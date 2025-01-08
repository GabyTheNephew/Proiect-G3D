#version 330 core
in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

out vec4 FragColor;

uniform sampler2D texture_diffuse1;
uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 lightColor;
uniform vec3 objectColor;
uniform sampler2D shadowMap;
uniform mat4 lightSpaceMatrix;

float ShadowCalculation(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir) {
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5; // Normalize to [0,1] range

    if (projCoords.z > 1.0) {
        return 0.0;
    }

    float shadow = 0.0;
    float bias = max(0.005 * (1.0 - dot(normal, lightDir)), 0.002);

    vec2 texelSize = 1.0 / textureSize(shadowMap, 0); // Shadow map texel size
    int pcfRange = 2; // Lower for performance, higher for smoothness

    for (int x = -pcfRange; x <= pcfRange; ++x) {
        for (int y = -pcfRange; y <= pcfRange; ++y) {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += (projCoords.z - bias > pcfDepth) ? 1.0 : 0.0;
        }
    }

    shadow /= float((pcfRange * 2 + 1) * (pcfRange * 2 + 1));

    return shadow;
}



void main()
{
    // Ambient
    float ambientStrength = 0.6;
    vec3 ambient = ambientStrength * lightColor;

    // Diffuse
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor * 0.8;

    // Specular
    float specularStrength = 0.5;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 16);
    vec3 specular = specularStrength * spec * lightColor;

    // Shadow Calculation
    vec4 FragPosLightSpace = lightSpaceMatrix * vec4(FragPos, 1.0);
    float shadow = ShadowCalculation(FragPosLightSpace, norm, lightDir);

    // Apply lighting
    vec3 lighting = ambient + (1.0 - shadow) * (diffuse + specular);
    vec3 color = lighting * texture(texture_diffuse1, TexCoords).rgb;

    FragColor = vec4(color, 1.0);
}