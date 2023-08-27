#version 330 
in vec2 TexCoords;
in vec2 v_position;
in vec3 v_color;
layout (location = 0) out vec4 color;
layout (location = 1) out vec4 f_alpha;  
uniform vec3 u_offset; //X = Y offset; Y = MaxDistance; Z MinDistance
uniform sampler2D u_texture;
uniform int u_samples = 2;
uniform vec3 textColor;
uniform float u_opacity = 1.0f;
uniform float u_textSize = 0.0f;
uniform vec2 u_screenRes = vec2(1600, 900);

void main()
{   
	if(u_offset.y > v_position.y)
	{
		discard;
	}
	if(u_offset.z < v_position.y)
	{
		discard;
	}
	float sampled = 0;
	vec2 texSize = vec2(clamp(1.0f - u_textSize / 2, 0, 1) / u_samples) / u_screenRes;
	for (int x = -u_samples / 2; x < u_samples / 2; x++)
	{
		for (int y = -u_samples / 2; y < u_samples / 2; y += 1)
		{
			sampled += texture(u_texture, TexCoords + (vec2(x, y) * texSize)).a;
		}
	}
	f_alpha.xyz = vec3(1);
	f_alpha.w = pow(sampled / (u_samples * u_samples), 0.9) * u_opacity;
	color = vec4(v_color, f_alpha.w);
}  