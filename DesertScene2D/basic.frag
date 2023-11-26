#version 330 core

in vec3 vertexColor;
out vec4 FragColor;

uniform vec4 uColor;
uniform vec4 uNightColor;
uniform sampler2D uTexture;
uniform bool useVertexColor;
uniform bool useTexture; // Flag to determine whether to use texturing

uniform bool isNight; 

void main() {
    if (useTexture) {
        vec4 texColor = texture(uTexture, vertexColor.xy);
        FragColor = texColor;
    } else if (useVertexColor) {
        FragColor = vec4(vertexColor, 1.0);
    } else {
        FragColor = isNight ? uNightColor : uColor; 
    }
}