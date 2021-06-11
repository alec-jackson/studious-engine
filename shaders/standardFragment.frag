#version 330 core

// Ouput data
in vec2 f_texcoord;
in float brightness;
out vec4 color;
uniform sampler2D mytexture;
uniform int hasTexture;
uniform float luminance;

in vec3 Color;

void main()
{
	// Output color = red
	if (hasTexture == 1) {
		vec4 texColor = texture(mytexture, f_texcoord);
		if (texColor.a < 0.1) {
			discard;
		}
		texColor[0] = texColor[0] * brightness * luminance;
		texColor[1] = texColor[1] * brightness * luminance;
		texColor[2] = texColor[2] * brightness * luminance;
		//texColor[3] = texColor[3] * brightness * luminance;
		color = texColor;
	//	color = texture(mytexture, f_texcoord);
	} else {
		color = vec4(Color, 1.0);
	}


}
