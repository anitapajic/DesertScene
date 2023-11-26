#version 330 core

in vec3 vertexColor;
out vec4 FragColor;

uniform vec4 uColor;
uniform sampler2D uTexture;
uniform bool useVertexColor;
uniform bool useTexture; // Flag to determine whether to use texturing

uniform bool uRenderStars;                // New uniform to check if we're rendering stars
uniform float uStarIntensity; 

void main() {
    if (useTexture) {
        vec4 texColor = texture(uTexture, vertexColor.xy);
        FragColor = texColor;
    } else if (useVertexColor) {
        FragColor = vec4(vertexColor, 1.0);
    }else if (uRenderStars) {
        FragColor = vec4(1.0, 1.0, 1.0, uStarIntensity); // White color with variable alpha for stars
    } else {
        FragColor = uColor; 
    }
}
