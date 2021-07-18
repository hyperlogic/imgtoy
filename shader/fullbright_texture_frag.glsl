uniform vec4 color;
uniform sampler2D colorTexture;

varying vec2 frag_uv;

void main(void)
{
    vec4 texColor = texture2D(colorTexture, frag_uv);

    // premultiplied alpha blending
    gl_FragColor.rgb = color.a * color.rgb * texColor.rgb;
    gl_FragColor.a = color.a * texColor.a;
}
