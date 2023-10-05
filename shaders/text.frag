#version 410

in vec2 TexCoords;
out vec4 fragColor;

layout(binding = 0) uniform sampler2D text;
uniform vec4 textColor; // Added alpha component for transparency

void main()
{    
    vec4 sampled = vec4(1.0, 1.0, 1.0, texture(text, TexCoords).r);
    fragColor = textColor * sampled; // Adjusted to include alpha component
}