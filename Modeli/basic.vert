#version 330 core
layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inUV;
layout(location = 3) in vec4 inCol; //Boja tjemena - ovo saljemo u fragment sejder
layout(location = 4) in vec2 inTex; 

out vec4 channelCol; 

out vec3 chFragPos;
out vec3 chNormal;
out vec2 TexCoords;

uniform vec3 uLightPos; 
uniform vec3 uViewPos; 
uniform vec3 uLightColor;

uniform mat4 uM;
uniform mat4 uV;
uniform mat4 uP;

uniform bool useTexture;
uniform bool useDesert;
uniform bool useWater;
uniform bool useFish;
uniform bool useGrass;
uniform bool usePyramidModel;


uniform mat4 rotationMatrix;
uniform mat4 translationMatrix;
uniform float scale;

uniform bool useGourad;
void main()
{

    // Perform lighting calculations (similar to those in the fragment shader)
    vec3 norm = normalize(chNormal);
    vec3 lightDir = normalize(uLightPos - chFragPos);
    vec3 viewDir = normalize(uViewPos - chFragPos);
    
    // Ambient
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * uLightColor;
    
    // Diffuse
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * uLightColor;
    
    // Specular
    float specularStrength = 0.5;
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * uLightColor;

   
    if (useTexture){
        gl_Position = vec4(inPos, 1.0);
        TexCoords = inTex;
    }
    else if (useDesert){
        TexCoords = inUV;
        mat4 scaledModel = uM * mat4(vec4(scale, 0.0, 0.0, 0.0),
                                    vec4(0.0, scale, 0.0, 0.0),
                                    vec4(0.0, 0.0, scale, 0.0),
                                    vec4(0.0, 0.0, 0.0, 1.0));

        mat4 finalModel = translationMatrix * scaledModel;
        chFragPos = vec3(finalModel * vec4(inPos, 1.0));
        chNormal = mat3(transpose(inverse(finalModel))) * inNormal;
        gl_Position = uP * uV * vec4(chFragPos, 1.0);
    }
    else if (useWater){
        TexCoords = inUV;
        mat4 scaledModel = uM * mat4(vec4(scale, 0.0, 0.0, 0.0),
                                    vec4(0.0, scale, 0.0, 0.0),
                                    vec4(0.0, 0.0, scale, 0.0),
                                    vec4(0.0, 0.0, 0.0, 1.0));
        mat4 finalModel = translationMatrix * scaledModel;
        chFragPos = vec3(finalModel * vec4(inPos, 1.0));
        chNormal = mat3(transpose(inverse(finalModel))) * inNormal;
        gl_Position = uP * uV * vec4(chFragPos, 1.0);
        if (useGourad){
             channelCol = vec4(0.0, 0.0, 1.0, 0.7) * vec4(ambient + diffuse + specular, 1.0);      
        }
        else{
            channelCol = vec4(0.0, 0.0, 1.0, 0.7);
        }
        
    }
    else if (useGrass){
        TexCoords = inUV;
        mat4 scaledModel = uM * mat4(vec4(scale, 0.0, 0.0, 0.0),
                                    vec4(0.0, scale, 0.0, 0.0),
                                    vec4(0.0, 0.0, scale, 0.0),
                                    vec4(0.0, 0.0, 0.0, 1.0));
        mat4 finalModel = translationMatrix * scaledModel;
        chFragPos = vec3(finalModel * vec4(inPos, 1.0));
        chNormal = mat3(transpose(inverse(finalModel))) * inNormal;
        gl_Position = uP * uV * vec4(chFragPos, 1.0);
        if (useGourad){
             channelCol = vec4(0.2, 1.0, 0.3, 0.5) * vec4(ambient + diffuse + specular, 1.0);      
        }
        else{
            channelCol = vec4(0.2, 1.0, 0.3, 0.5);
        }
        
    }
    else if(useFish){
        TexCoords = inUV;
        mat4 scaledModel = uM * mat4(vec4(scale, 0.0, 0.0, 0.0),
                                    vec4(0.0, scale, 0.0, 0.0),
                                    vec4(0.0, 0.0, scale, 0.0),
                                    vec4(0.0, 0.0, 0.0, 1.0));
        mat4 finalModel = translationMatrix * rotationMatrix * scaledModel;
        chFragPos = vec3(finalModel * vec4(inPos, 1.0));
        chNormal = mat3(transpose(inverse(finalModel))) * inNormal;
        gl_Position = uP * uV * vec4(chFragPos, 1.0);
        if (useGourad){
             channelCol = vec4(1.0, 0.0, 0.0, 1.0) * vec4(ambient + diffuse + specular, 1.0);      
        }
        else{
            channelCol = vec4(1.0, 0.0, 0.0, 1.0);
        }
    }
    else if (usePyramidModel){
        TexCoords = inUV;
        mat4 finalModel = translationMatrix;
        chFragPos = vec3(finalModel * vec4(inPos, 1.0));
        chNormal = mat3(transpose(inverse(finalModel))) * inNormal;
        gl_Position = uP * uV * vec4(chFragPos, 1.0);     
    }  
}

