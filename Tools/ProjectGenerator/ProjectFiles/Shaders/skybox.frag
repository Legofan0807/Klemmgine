//! #include "shared.frag"

// #params colors
uniform vec3 u_skycolor = vec3(0.3, 0.6, 1);
uniform vec3 u_horizoncolor = vec3(0.7, 0.8, 1);

void main()
{
	vec3 normal = normalize(-v_position);
	vec3 color = mix(u_horizoncolor, u_skycolor, abs(dot(vec3(normalize(normal)), vec3(0, 1, 0))));
	if (max(pow(dot(normalize(-normal), normalize(u_directionallight.Direction)), 100), 0) > 0.9)
	{
		color = u_directionallight.SunColor * 2;
	}
	RETURN_COLOR_NO_FOG(color)
}