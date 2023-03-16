#version 410 core

in vec3 fPosition;
in vec3 fNormal;
in vec2 fTexCoords;
in vec4 fragPosLightSpace;

//ceata
in vec3 fragPos;
in vec4 fPosEye;

out vec4 fColor;

//matrices
uniform mat4 model;
uniform mat4 view;
uniform mat3 normalMatrix;
//lighting
uniform vec3 lightDir;
uniform vec3 lightColor;
// textures
uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;
uniform sampler2D shadowMap;

//components
vec3 ambient;
float ambientStrength = 0.2f;
vec3 diffuse;
vec3 specular;
float specularStrength = 0.5f;

float computeShadow(){
	
	// perform perspective divide
	vec3 normalizedCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;

	// Transform to [0,1] range
	normalizedCoords = normalizedCoords * 0.5 + 0.5;
	
	if (normalizedCoords.z > 1.0f)
		return 0.0f;

	// Get closest depth value from light's perspective
	float closestDepth = texture(shadowMap, normalizedCoords.xy).r;

	// Get depth of current fragment from light's perspective
	float currentDepth = normalizedCoords.z;

	// Check whether current frag pos is in shadow
	float bias = 0.005f;
	float shadow = currentDepth - bias > closestDepth ? 1.0f : 0.0f;

	return shadow;
}

void computeDirLight(){
    //compute eye space coordinates
    vec3 normalEye = normalize(normalMatrix * fNormal);

    //normalize light direction
   // vec3 lightDirN = vec3(normalize(view * vec4(lightDir, 0.0f)));
    vec3 lightDirN = normalize(lightDir);
    //compute view direction (in eye coordinates, the viewer is situated at the origin
    vec3 viewDir = normalize(- fPosEye.xyz);

    //compute ambient light
    ambient = ambientStrength * lightColor;

    //compute diffuse light
    diffuse = max(dot(normalEye, lightDirN), 0.0f) * lightColor;

    //compute specular light
    vec3 reflectDir = reflect(-lightDirN, normalEye);
    float specCoeff = pow(max(dot(viewDir, reflectDir), 0.0f), 32);
    specular = specularStrength * specCoeff * lightColor;
    ambient *= texture(diffuseTexture, fTexCoords).rgb;
	diffuse *= texture(diffuseTexture, fTexCoords).rgb;
	specular *= texture(specularTexture, fTexCoords).rgb;

}

float computeFog(){
     float fogDensity = 0.05f;
     float fragmentDistance = length(fPosEye);
     float fogFactor = exp(-pow(fragmentDistance * fogDensity, 2));
 
     return clamp(fogFactor, 0.0f, 1.0f);
}

void main(){
    computeDirLight();

    vec4 colorFromTexture = texture(diffuseTexture, fTexCoords);
    if(colorFromTexture.a < 0.1)
        discard;

    float shadow = computeShadow();
    vec3 color = min((ambient + (1.0f - shadow)*diffuse) + (1.0f - shadow)*specular, 1.0f);

    float fogFactor = computeFog();
    vec4 fogColor = vec4(0.5f, 0.5f, 0.5f, 1.0f);

    vec4 color2 = vec4(color, 1.0f) + (0.2 * colorFromTexture);

    //compute final vertex color
   fColor = mix(fogColor, color2, fogFactor);
}
