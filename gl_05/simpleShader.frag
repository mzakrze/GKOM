#version 330 core
in vec2 TexCoord;

out vec4 color;

uniform sampler2D Texture0;

void main()
{
	color = texture2D(Texture0, TexCoord);
}
