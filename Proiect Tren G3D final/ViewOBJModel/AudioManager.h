#pragma once
#include <al.h>
#include <alc.h>
#include <vector>
#include <string>
#include <Windows.h>
#include <iostream>

class AudioManager {
private:
    ALCdevice* device;
    ALCcontext* context;
    std::vector<ALuint> buffers;
    std::vector<ALuint> sources;

    bool loadWavFile(const char* filename, ALuint buffer) {
        FILE* file = nullptr;
        fopen_s(&file, filename, "rb");
        if (!file) {
            std::cout << "Cannot open sound file!" << std::endl;
            return false;
        }

        try {
            // WAV headers
            char type[4];
            DWORD size, chunkSize;
            short formatType, channels;
            DWORD sampleRate, avgBytesPerSec;
            short bytesPerSample, bitsPerSample;
            DWORD dataSize;

            fread(type, sizeof(char), 4, file);
            if (type[0] != 'R' || type[1] != 'I' || type[2] != 'F' || type[3] != 'F') {
                std::cout << "Invalid WAV file format!" << std::endl;
                fclose(file);
                return false;
            }

            fread(&size, sizeof(DWORD), 1, file);
            fread(type, sizeof(char), 4, file);
            if (type[0] != 'W' || type[1] != 'A' || type[2] != 'V' || type[3] != 'E') {
                fclose(file);
                return false;
            }

            // Citește chunk-ul fmt
            bool foundFmt = false;
            bool foundData = false;

            while (!foundData && !feof(file)) {
                fread(type, sizeof(char), 4, file);
                fread(&chunkSize, sizeof(DWORD), 1, file);

                if (memcmp(type, "fmt ", 4) == 0) {
                    foundFmt = true;
                    fread(&formatType, sizeof(short), 1, file);
                    fread(&channels, sizeof(short), 1, file);
                    fread(&sampleRate, sizeof(DWORD), 1, file);
                    fread(&avgBytesPerSec, sizeof(DWORD), 1, file);
                    fread(&bytesPerSample, sizeof(short), 1, file);
                    fread(&bitsPerSample, sizeof(short), 1, file);

                    // Skip any extra format bytes
                    if (chunkSize > 16) {
                        fseek(file, chunkSize - 16, SEEK_CUR);
                    }
                }
                else if (memcmp(type, "data", 4) == 0) {
                    foundData = true;
                    dataSize = chunkSize;
                }
                else {
                    // Skip unknown chunk
                    fseek(file, chunkSize, SEEK_CUR);
                }
            }

            if (!foundFmt || !foundData) {
                std::cout << "Invalid WAV format!" << std::endl;
                fclose(file);
                return false;
            }

            // Alocă memorie pentru date audio
            std::vector<unsigned char> buffer_data(dataSize);
            fread(buffer_data.data(), sizeof(unsigned char), dataSize, file);

            fclose(file);

            // Set OpenAL buffer format
            ALenum format;
            if (channels == 1) {
                format = (bitsPerSample == 8) ? AL_FORMAT_MONO8 : AL_FORMAT_MONO16;
            }
            else {
                format = (bitsPerSample == 8) ? AL_FORMAT_STEREO8 : AL_FORMAT_STEREO16;
            }

          
            alGetError(); 
            alBufferData(buffer, format, buffer_data.data(), dataSize, sampleRate);

            ALenum error = alGetError();
            if (error != AL_NO_ERROR) {
                std::cout << "Error loading buffer data: " << error << std::endl;
                return false;
            }

            return true;
        }
        catch (const std::exception& e) {
            std::cout << "Exception while loading WAV file: " << e.what() << std::endl;
            if (file) fclose(file);
            return false;
        }
    }

public:
    AudioManager() : device(nullptr), context(nullptr) {
        device = alcOpenDevice(nullptr);
        if (!device) {
            std::cout << "Failed to open audio device!" << std::endl;
            return;
        }

        context = alcCreateContext(device, nullptr);
        if (!context) {
            std::cout << "Failed to create audio context!" << std::endl;
            alcCloseDevice(device);
            return;
        }

        if (!alcMakeContextCurrent(context)) {
            std::cout << "Failed to make context current!" << std::endl;
            alcDestroyContext(context);
            alcCloseDevice(device);
            return;
        }

        alGetError();

        std::cout << "Audio Manager initialized successfully" << std::endl;
        std::cout << "OpenAL Vendor: " << alGetString(AL_VENDOR) << std::endl;
        std::cout << "OpenAL Version: " << alGetString(AL_VERSION) << std::endl;
        std::cout << "OpenAL Renderer: " << alGetString(AL_RENDERER) << std::endl;
    }

    ~AudioManager() {
 
        for (ALuint source : sources) {
            alSourceStop(source);
            alDeleteSources(1, &source);
        }
        sources.clear();

        for (ALuint buffer : buffers) {
            alDeleteBuffers(1, &buffer);
        }
        buffers.clear();

        if (context) {
            alcMakeContextCurrent(nullptr);
            alcDestroyContext(context);
        }
        if (device) {
            alcCloseDevice(device);
        }
    }

    bool isInitialized() const {
        return device != nullptr && context != nullptr;
    }

    ALuint loadSound(const char* filename) {

        alGetError();

        ALuint buffer;
        alGenBuffers(1, &buffer);

        ALenum error = alGetError();
        if (error != AL_NO_ERROR) {
            std::cout << "Error generating buffer: " << error << std::endl;
            return 0;
        }

        if (loadWavFile(filename, buffer)) {
            buffers.push_back(buffer);

            while (sources.size() >= 16) { // OpenAL are de obicei o limită de 16 surse
                ALuint oldSource = sources.back();
                alDeleteSources(1, &oldSource);
                sources.pop_back();
            }

            // Creează noua sursă
            ALuint source;
            alGenSources(1, &source);

            error = alGetError();
            if (error != AL_NO_ERROR) {
                std::cout << "Error generating source: " << error << std::endl;
                alDeleteBuffers(1, &buffer);
                return 0;
            }
            alSourcei(source, AL_BUFFER, buffer);
            alSourcef(source, AL_GAIN, 1.0f);
            alSourcef(source, AL_PITCH, 1.0f);
            alSource3f(source, AL_POSITION, 0.0f, 0.0f, 0.0f);
            alSource3f(source, AL_VELOCITY, 0.0f, 0.0f, 0.0f);

            sources.push_back(source);
            return source;
        }

        alDeleteBuffers(1, &buffer);
        return 0;
    }

    void playSound(ALuint source) {
        ALint state;
        alGetSourcei(source, AL_SOURCE_STATE, &state);

        // Verifică dacă sursa este validă
        ALenum error = alGetError();
        if (error != AL_NO_ERROR) {
            std::cout << "OpenAL Error before playing: " << error << std::endl;
            return;
        }

        alSourcePlay(source);

        // Verifică dacă redarea a început cu succes
        error = alGetError();
        if (error != AL_NO_ERROR) {
            std::cout << "OpenAL Error after playing: " << error << std::endl;
        }
    }

    void stopSound(ALuint source) {
        alSourceStop(source);
        ALenum error = alGetError();
        if (error != AL_NO_ERROR) {
            std::cout << "Error stopping sound: " << error << std::endl;
        }
    }

    void setLooping(ALuint source, bool loop) {
        alSourcei(source, AL_LOOPING, loop ? AL_TRUE : AL_FALSE);
        ALenum error = alGetError();
        if (error != AL_NO_ERROR) {
            std::cout << "Error setting looping: " << error << std::endl;
        }
    }

    void setPitch(ALuint source, float pitch) {
        alSourcef(source, AL_PITCH, pitch);
        ALenum error = alGetError();
        if (error != AL_NO_ERROR) {
            std::cout << "Error setting pitch: " << error << std::endl;
        }
    }

    void setVolume(ALuint source, float volume) {
        alSourcef(source, AL_GAIN, volume);
        ALenum error = alGetError();
        if (error != AL_NO_ERROR) {
            std::cout << "Error setting volume: " << error << std::endl;
        }
    }

    bool isPlaying(ALuint source) {
        ALint state;
        alGetSourcei(source, AL_SOURCE_STATE, &state);
        ALenum error = alGetError();
        if (error != AL_NO_ERROR) {
            std::cout << "Error checking play state: " << error << std::endl;
            return false;
        }
        return state == AL_PLAYING;
    }
};