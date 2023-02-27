#version 450                          
out vec4 FragColor;

in struct Vertex {
    vec3 WorldNormal;
    vec3 WorldPosition;
}v_out;

struct Camera {
    vec3 cameraPos;
    vec3 cameraDir;
};

struct Material {
	vec3 color;
	float ambientK;
	float diffuseK;
	float specularK;
	float shininess;
};

struct Directional {
    vec3 color;
    vec3 direction;
    float intensity;
};

struct Point {
    vec3 color;
    vec3 position;
    float intensity;
    //lin_attenuation
    float linear;
    float quadratic;
    float range;
};

struct Spot {
    vec3 color;
    vec3 position;
    vec3 direction;
    float intensity;
    //lin_attenuation
    float min_angle;
    float max_angle;
    float angularFallOff;
    float range;
};

#define MAX_LIGHTS 3
uniform Directional dirLight;
uniform Point pointLights[MAX_LIGHTS];
uniform Spot spotLight;
uniform Material material;
uniform Camera camera;

float calcAmbient(float intensity) {
    return material.ambientK * intensity;
}

float calcDiffuse(float intensity, vec3 lightDir)
{
    vec3 normal = normalize(v_out.WorldNormal);
    return material.diffuseK * max(dot(lightDir, normal), 0.0) * intensity;
}

float calcSpecular(float intensity, vec3 reflectDir)
{
    vec3 viewDir = normalize(camera.cameraPos - v_out.WorldPosition);
    return material.specularK * pow(max(dot(reflectDir, viewDir), 0.0), material.shininess) * intensity;
}

vec3 calcDirLight (Directional light, vec3 normal) {
    vec3 lightDir = normalize(-light.direction);
    vec3 reflectDir = reflect(-lightDir, normal);

    float ambient = calcAmbient(light.intensity);
    float diffuse = calcDiffuse(light.intensity, lightDir);
    float specular = calcSpecular(light.intensity, reflectDir);

    return (ambient + diffuse + specular) * light.color;
}

vec3 calcPointLight (Point light, vec3 normal) {
    vec3 lightDir = light.position - v_out.WorldPosition;
    float dist = length(lightDir);
    lightDir = normalize(lightDir);
    vec3 reflectDir = reflect(-lightDir, normal);

    float attentuation = 1.0 / (1.0 + light.linear * dist + light.quadratic * (dist * dist));

    float ambient = calcAmbient(light.intensity);
    float diffuse = calcDiffuse(light.intensity, lightDir);
    float specular = calcSpecular(light.intensity, reflectDir);

    return vec3(light.color) * (ambient + diffuse + specular) * attentuation * light.range;
}

vec3 calcSpotLight (Spot light, vec3 normal, vec3 viewDir) {
    vec3 lightDir = normalize(light.position - v_out.WorldPosition);
    vec3 reflectDir = reflect(-lightDir, normal);

    float ambient = calcAmbient(light.intensity);
    float diffuse = calcDiffuse(light.intensity, lightDir);
    float specular = calcSpecular(light.intensity, reflectDir);

    float theta = dot(-lightDir, normalize (light.direction));
    float epsilon = light.min_angle - light.max_angle;
    float attenuation = pow(clamp((theta - light.max_angle) / epsilon, 0.0, 1.0), light.angularFallOff);

    return vec3(light.color) * (ambient + diffuse + specular) * attenuation * light.range;
}

void main(){   
    vec3 normal = normalize(v_out.WorldNormal);

    vec3 cameraDirection = normalize(camera.cameraPos - v_out.WorldPosition);

    vec3 allLights = calcDirLight(dirLight, normal);

    for (int i = 0; i < MAX_LIGHTS; i++) {
        allLights += calcPointLight(pointLights[i], normal);
    }

    allLights += calcSpotLight(spotLight, normal, camera.cameraDir);

    FragColor = vec4(allLights * material.color, 1.0f);
}
