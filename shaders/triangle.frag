#version 330 core
out vec4 FragColor;

in vec3 ourColor;
in vec2 texCoord;

uniform sampler2D ourTexture;
uniform vec4 rainbowColor;

void main() {
    FragColor = texture(ourTexture, texCoord) * vec4(ourColor, 1.0) * rainbowColor;
}
