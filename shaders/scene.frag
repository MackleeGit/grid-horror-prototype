#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec2 TexCoord;

uniform sampler2D texture1;

// Flashlight / Spotlight uniforms
uniform vec3 lightPos;
uniform vec3 lightDir;
uniform float cutOff;
uniform float outerCutOff;

// Distance quadratic attenuation factors
uniform float constant;
uniform float linear;
uniform float quadratic;

// Atmospheric horror parameters
uniform float ambientStrength;
uniform float flickerFactor;

void main()
{
    vec3 texColor = texture(texture1, TexCoord).rgb;
    vec3 lightToFrag = normalize(lightPos - FragPos);

    // Minimum ambient baseline
    vec3 ambient = ambientStrength * texColor;

    // Spotlight cone cosine calculation
    float theta = dot(lightToFrag, normalize(-lightDir));
    float epsilon = cutOff - outerCutOff;
    float intensity = clamp((theta - outerCutOff) / epsilon, 0.0, 1.0);

    // Distance quadratic attenuation
    float distance = length(lightPos - FragPos);
    float attenuation = 1.0 / (constant + linear * distance + quadratic * (distance * distance));

    // Combined lighting with dynamic flicker modulation
    vec3 spotlight = texColor * intensity * attenuation * flickerFactor;

    FragColor = vec4(ambient + spotlight, 1.0);
}