#version 330 core
layout (location = 0) in vec2 aPos;        // Vertex position
layout (location = 1) in vec3 aColor;      // Vertex color
layout (location = 2) in vec2 aTexCoord;   // Texture coordinates

out vec3 vertexColor;
out vec2 TexCoord;
uniform vec2 uSunPos;                      // Pozicija Sunca
uniform vec2 uMoonPos;                     // Pozicija polumeseca
uniform bool uIsSun;                       // Da li se crta Sunce
uniform bool uUseSunMoonPositioning;       // Da li koristiti posebno pozicioniranje za Sunce i polumesec

void main() {
    vec2 position = aPos;

    if (uUseSunMoonPositioning) {
        if (uIsSun) {
            position += uSunPos; // Dodajemo poziciju Sunca
        } else {
            position += uMoonPos; // Dodajemo poziciju polumeseca
        }
    }

    gl_Position = vec4(position, 0.0, 1.0); // Convert the vec2 position to a vec4 with z = 0 and w = 1
    vertexColor = aColor;                   // Pass the color to the fragment shader
    TexCoord = aTexCoord;                   // Pass the texture coordinates to the fragment shader
}
