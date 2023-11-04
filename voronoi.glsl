#version 300 es

precision highp float;
precision highp sampler2D;

in vec2 uv;
out vec4 out_color;


// Generiert für übergebene number jeweils den selben Farbwert
vec3 randColor(int number){
    return fract(sin(vec3(number+1)*vec3(12.8787, 19.97, 20.73739)));
}

float random(vec2 vec) {
    return fract(sin(dot(vec.xy, vec2(233.33322, 122.233382)))*41422.232333);
}

void main() {

    vec3 col = vec3(1); 
    float cellcore_radius = 0.01;
    
    const int points_length = 25; 
    vec2 points_array[points_length];


    
    float dmin = 1000.0; 
    int point = 0;
    
    for(int i = 0; i < points_length; i++){

        vec2 pos = vec2(
          random(vec2(i+1)),
          random(vec2(i+1)*0.5*cos(float(i)*.2387))
        );

        points_array[i] = pos;

        vec2 uv_rounded = floor(uv*100.0)/100.0;
        vec2 point_rounded = floor(points_array[i]*100.0)/100.0;
        
        if(length(uv-pos) <= cellcore_radius){
            col = vec3(0); 
        }
        if(length(points_array[i] - uv) < dmin){
            point = i; 
            dmin = length(points_array[i] - uv); 
        }
    }

    // Output to screen
    out_color = vec4((randColor(point)-dmin)*col,1.0);
}
