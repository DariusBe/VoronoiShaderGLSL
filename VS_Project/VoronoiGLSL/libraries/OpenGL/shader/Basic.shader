#shader vertex
#version 330 core

layout (location = 0) in vec4 position;
void main (void)
{
  gl_Position = position;
}


#shader fragment
#version 330 core

out vec4 out_color;

#define max_input_points 100

uniform int inputPointsCount;
uniform vec2 inputPoints[max_input_points];

uniform vec2 resolution; // Screen resolution from screen size
uniform vec2 mousePos; // Mouse position


float random(vec2 vec) {
    return fract(sin(dot(vec.xy, vec2(233.33322, 122.233382))) * 41422.232333);
}

vec3 randColor(int number) {
    return fract(sin(vec3(number + 1) * vec3(12.8787, 19.97, 20.73739)));
}

// Function to create a smoother transition between colors
float smoothEdge(float distance, float edgeWidth) {
    return smoothstep(0.2, edgeWidth, distance);
}

void main() {
    vec3 background = vec3(1.0);
    vec2 uv = gl_FragCoord.xy / resolution;

    // Check if the fragment position is close to the mouse position
    float distance = distance(uv, mousePos);
    float cellcore_radius = 0.005;

    int displayed_points = 25 + inputPointsCount;
    vec2 points_array[25 + max_input_points];

    float dmin = 10.0;
    float edgeWidth = 0.00002;
    int point = 0;

    for (int i = 0; i < displayed_points; i++) {
        points_array[25+i] = vec2(inputPoints[i]);
    }

    //fill array with points, randomly:
    for (int i = 0; i < 25; i++) {
        vec2 pos = vec2(
            random(vec2(i + 1)),
            random(vec2(i + 1) * 0.5 * cos(float(i) * .2387))
        );
        points_array[i] = pos;
    }

    //display points:
    for (int i = 0; i < displayed_points; i++) {
        if (distance(uv, points_array[i]) <= cellcore_radius) {
            background = vec3(0);
        }
        if (distance(uv, points_array[i]) < dmin) {
            point = i;
            dmin = length(uv - points_array[i]);
            //edgeWidth=dmin;
        }
    }
    vec3 shading = mix(randColor(point), vec3(1.0), smoothEdge(dmin, edgeWidth));

    out_color = vec4(shading * background, 1.0);

}