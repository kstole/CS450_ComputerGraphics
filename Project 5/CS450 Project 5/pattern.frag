#version 330 //compatibility

uniform float	uTime;		// "Time", from Animate( )
in vec2  	vST;		// texture coords

void main( ) {
	vec3 myColor = vec3( 1, 1, 1 );
	if (true) {
		myColor = vec3( 1, 1, 1 );
	}
	gl_FragColor = vec4( myColor,  1. );
}
