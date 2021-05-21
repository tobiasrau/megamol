
layout(std430, binding = 0) readonly buffer MeshShaderParamsBuffer { MeshShaderParams[] mesh_shader_params; };

layout(std430, binding = 1) readonly buffer PerFrameDataBuffer { PerFrameData[] per_frame_data; };

uniform mat4 view_mx;

layout(location = 0) flat in int draw_id;
layout(location = 1) in vec2 uv_coords;
layout(location = 2) in vec3 pixel_vector;
layout(location = 3) in vec3 cam_vector;

layout(location = 0) out vec4 albedo_out;
layout(location = 1) out vec3 normal_out;
layout(location = 2) out float depth_out;
layout(location = 3) out int objID_out;
layout(location = 4) out vec4 interactionData_out;

#define PI 3.1415926

vec3 fakeViridis(float lerp)
{
    vec3 c0 = vec3(0.2823645529290169,0.0,0.3310101940118055);
    vec3 c1 = vec3(0.24090172204161298,0.7633448774061599,0.42216355577803744);
    vec3 c2 = vec3(0.9529994532916154,0.9125452328290099,0.11085876909361342);

    return lerp < 0.5 ? mix(c0,c1,lerp * 2.0) : mix(c1,c2,(lerp*2.0)-1.0);
};

vec3 projectOntoPlane(vec3 v, vec3 n)
{
    return ( v - (( dot(v,n) / length(n) ) * n) );
};

void main() {

    if(dot(cam_vector,mesh_shader_params[draw_id].probe_direction.xyz) < 0.0 ){
        discard;
    }

    vec4 glyph_border_color = vec4(1.0);

    if(mesh_shader_params[draw_id].state == 1) {
        glyph_border_color = vec4(1.0,1.0,0.0,1.0);
    }
    else if(mesh_shader_params[draw_id].state == 2) {
        glyph_border_color = vec4(1.0,0.58,0.0,1.0);
    }

    // Highlight glyph up and glyph right directions
    if( (uv_coords.x > 0.99 && uv_coords.x > uv_coords.y && uv_coords.y > 0.9) ||
        (uv_coords.y > 0.99 && uv_coords.x < uv_coords.y && uv_coords.x > 0.9) ||
        (uv_coords.x < 0.01 && uv_coords.x < uv_coords.y && uv_coords.y < 0.05) ||
        (uv_coords.y < 0.01 && uv_coords.x > uv_coords.y && uv_coords.x < 0.05) )
    {
        albedo_out = glyph_border_color;
        normal_out = vec3(0.0,0.0,1.0);
        depth_out = gl_FragCoord.z;
        objID_out = mesh_shader_params[draw_id].probe_id;
        return;
    }
    
    float r = length(uv_coords - vec2(0.5)) * 2.0;

    if(r > 1.0) discard;
    if(r < 0.1) discard;

    float border_circle_width = 0.02;
    if(mesh_shader_params[draw_id].state == 1) {
        border_circle_width = 0.06;
    }
    else if(mesh_shader_params[draw_id].state == 2) {
        border_circle_width = 0.06;
    }

    if(r > (1.0 - border_circle_width)){
        albedo_out = glyph_border_color;
        normal_out = vec3(0.0,0.0,1.0);
        depth_out = gl_FragCoord.z;
        objID_out = mesh_shader_params[draw_id].probe_id;
        return;
    }

    float radar_sections_cnt = mesh_shader_params[draw_id].sample_cnt;
    vec3 proj_pv = normalize(projectOntoPlane(pixel_vector,mesh_shader_params[draw_id].probe_direction.xyz));
    
    vec3 out_colour = vec3(0.0,0.0,0.0);
    bool interpolate = bool(per_frame_data[0].use_interpolation);

    vec3 sample_vector = vec3(0.0);
    float sample_magnitude = 0.0;

    // inverse direction of sample lookup to map higher sample depth to smaller radius
    // also shift slightly away from probe center, since that region is not useful to clearly show directions
    float invere_r = 1.0 - ( (r - 0.1) / 0.9 );

    if(interpolate)
    {    
        // identify section of radar glyph that the pixel belongs to
        int radar_section_0 = clamp(int(floor(invere_r * (radar_sections_cnt - 1.0))),0,int(radar_sections_cnt)-1);
        int radar_section_1 = clamp(int(ceil(invere_r * (radar_sections_cnt - 1.0))),0,int(radar_sections_cnt)-1);
        float lerp = fract(invere_r * (radar_sections_cnt-1));

        // based on section, calculate vector projection
        vec3 sample_vector_0 = normalize(mesh_shader_params[draw_id].samples[radar_section_0].xyz);
        float sample_magnitude_0 = mesh_shader_params[draw_id].samples[radar_section_0].w;

        vec3 sample_vector_1 = normalize(mesh_shader_params[draw_id].samples[radar_section_1].xyz);
        float sample_magnitude_1 = mesh_shader_params[draw_id].samples[radar_section_1].w;

        sample_vector = normalize(mix(sample_vector_0,sample_vector_1,lerp));
        sample_magnitude = mix(sample_magnitude_0,sample_magnitude_1,lerp);
    }
    else
    {
        int radar_section = clamp(int(round(invere_r * (radar_sections_cnt - 1.0))),0,int(radar_sections_cnt)-1);

        sample_vector = normalize(mesh_shader_params[draw_id].samples[radar_section].xyz);
        sample_magnitude = mesh_shader_params[draw_id].samples[radar_section].w;
    }

    if( isnan(sample_magnitude) ) discard;

    //discard invalid samples
    if(sample_magnitude < 0.005 && sample_magnitude > -0.005) discard;

    vec3 proj_sv =  normalize(projectOntoPlane(sample_vector,mesh_shader_params[draw_id].probe_direction.xyz));

    float proj_sample_dot_pixel = dot(proj_sv,proj_pv);
    float sample_dot_probe = dot(sample_vector,mesh_shader_params[draw_id].probe_direction.xyz);

    float circumference = 2.0 * PI * r;
    float inner_angle = acos(proj_sample_dot_pixel);

    float arc_length = (inner_angle / (2.0*PI)) * 2.0 * circumference;
    float tgt_arc_length = ( 1.0 - (acos(abs(sample_dot_probe)) / (0.5*PI)) ) * circumference;

    float eps = -0.05;
    if( (arc_length + eps) > tgt_arc_length ) discard;

    sampler2D tf_tx = sampler2D(mesh_shader_params[draw_id].tf_texture_handle);
    float tf_min = mesh_shader_params[draw_id].tf_min;
    float tf_max = mesh_shader_params[draw_id].tf_max;
    out_colour = texture(tf_tx, vec2((sample_magnitude - tf_min) / (tf_max-tf_min), 0.5) ).rgb;
    //out_colour = fakeViridis( (sample_magnitude + 2.0) / 16.0);

    albedo_out = vec4(out_colour,1.0);
    normal_out = vec3(0.0,0.0,1.0);
    depth_out = gl_FragCoord.z;

    objID_out = mesh_shader_params[draw_id].probe_id;
    interactionData_out = vec4(0.0);
}