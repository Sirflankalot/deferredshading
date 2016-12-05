#version 330 core

out float FragColor;

in vec2 vTexCoords;

uniform sampler2D gPositionDepth;
uniform sampler2D gNormal;
uniform sampler2D texNoise;

uniform vec3 samples[64];
uniform mat4 view;
uniform mat4 projection;

const int kernelSize = 64;
const float radius = 1.0;

void main() {
	// Inputs
	vec3 fragPos = vec3(view * vec4(texture(gPositionDepth, vTexCoords).xyz, 1.0));
	vec3 normal = mat3(transpose(inverse(view))) * texture(gNormal, vTexCoords).rgb;
	vec3 randomVec = texture(texNoise, gl_FragCoord.xy / vec2(4.0)).xyz;

	// Create TBN change-of-basis matrix: from tangent-space to view-space
	vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
	vec3 bitangent = cross(normal, tangent);
	mat3 TBN = mat3(tangent, bitangent, normal);

	// Iterate over the sample kernel and calculate occlusion factor
    float occlusion = 0.0;
    for(int i = 0; i < kernelSize; ++i) {
        // get sample position
        vec3 sample = TBN * samples[i]; // From tangent to view-spaaaaaace
        sample = fragPos + sample * radius;
        
        // project sample position (to sample texture) (to get position on screen/texture)
        vec4 offset = vec4(sample, 1.0);
        offset = projection * offset; // from view to clip-spaaaaaace
        offset.xyz /= offset.w; // perspective divide
        offset.xyz = offset.xyz * 0.5 + 0.5; // transform to range 0.0 - 1.0
        
        // get sample depth
        float sampleDepth = -texture(gPositionDepth, offset.xy).w; // Get depth value of kernel sample
        
        // range check & accumulate
        float rangeCheck = smoothstep(0.0, 1.0, radius / abs(fragPos.z - sampleDepth ));
        float final = (sampleDepth >= sample.z ? 1.0 : 0.0) * rangeCheck;
        occlusion += final;
    }
    occlusion = 1.0 - (occlusion / kernelSize);
    
    FragColor = pow(occlusion, 15);
}
