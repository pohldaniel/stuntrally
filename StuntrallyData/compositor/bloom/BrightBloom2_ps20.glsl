//-------------------------------
//BrightBloom_ps20.glsl
// High-pass filter for obtaining lumminance
// We use an aproximation formula that is pretty fast:
//   f(x) = ( -3 * ( x - 1 )^2 + 1 ) * 2
//   Color += Grayscale( f(Color) ) + 0.6
//
// Special thanks to ATI for their great HLSL2GLSL utility
//     https://sourceforge.net/projects/hlsl2glsl
//-------------------------------

uniform sampler2D RT;
varying vec2 uv;

void main()
{
    vec4 tex;
    vec4 bright4;
    float bright;
    
    tex = texture2D( RT, uv);
    tex -= 1.00000;
    bright4 = -6.00000 * tex * tex + 2.00000;
    bright = dot( bright4, vec4( 0.333333, 0.333333, 0.333333, 0.000000) );
    tex += (bright + 0.600000);

    gl_FragColor = tex;
}
