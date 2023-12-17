#version 300 es

precision highp float;
precision highp sampler2D;

in vec2 uv;
out vec4 out_color;
uniform vec2 u_resolution;
uniform vec4 u_mouse;


// generate same color value for passed point (index)
vec3 randColor(int number) {
    return fract(sin(vec3(number+1)*vec3(12.8787, 19.97, 20.73739)));
}

float random(vec2 vec) {
    return fract(sin(dot(vec.xy, vec2(233.33322, 122.233382)))*41422.232333);
}

// Function to create a smoother transition between colors
float smoothEdge(float distance, float edgeWidth) {
    return smoothstep(0.2, edgeWidth, distance);
}

// Function to return point at cursor position
vec2 getPointAtCursor(vec2 pos, vec2 mouse) {
        if(u_mouse.p == 1.) {
            pos.x = vec2(mouse).x;
            pos.y = 1.-vec2(mouse).y;
        }
        return pos;
}

void main() {
    vec2 mouse = u_mouse.xy / u_resolution;
    vec3 col = vec3(1);
    float cellcore_radius = 0.005;

    const int points_length = 35;
    vec2 points_array[points_length];



    float dmin = 10.0;
    float edgeWidth = 0.00002;
    int point = 0;

    for(int i = 0; i < points_length; i++) {
        vec2 pos = vec2(
          random(vec2(i+1)),
          random(vec2(i+1)*0.5*cos(float(i)*.2387))
        );

        points_array[i] = pos;

        pos = getPointAtCursor(pos, mouse);

        if(length(uv-pos) <= cellcore_radius){
            col = vec3(0);
        }
        if(length(uv-pos) < dmin) {
            point = i;
            dmin = length(uv-pos);
            //edgeWidth=dmin;
        }
    }
    vec3 shading = mix(randColor(point), vec3(1.0), smoothEdge(dmin, edgeWidth));

    // Output to screen
    out_color = vec4(shading*col, 1.0);
    //out_color = vec4((randColor(point)-(8.dmin))*col,1.0);
}