#version 300 es

precision highp float;
precision highp sampler2D;

in vec2 uv;
out vec4 out_color;

uniform vec2 u_resolution;
uniform float u_time;
uniform vec4 u_mouse;

float random(vec2 vec) {
    return fract(sin(dot(vec.xy, vec2(233.33322, 122.233382)))*41422.232333);
}

vec2 getPointAtCursor(vec2 pos, vec2 mouse) {
        if(u_mouse.p == 1.) { 
            pos.x = vec2(mouse).x; 
            pos.y = 1.-vec2(mouse).y; 
        }
        return pos;
}

void main() {

    vec2 mouse = u_mouse.xy / u_resolution;
    float light = 1.0;
    float cellcore_radius = 0.01;

    vec3 col = vec3(0);
    int cell_count = 15;
    vec3 cells = vec3(1);


    for(int i=0; i<cell_count; i++) {

        vec2 pos = vec2(
            random(vec2(i+1)),
            random(vec2(i+1)*0.5*cos(float(i)*.2387))
        );

        pos = getPointAtCursor(pos, mouse); //Setzt (bei gehaltener Maus) Voronoi-Zelle an Cursor 
        

        pos.x = min(pos.x, 1.);
        pos.y = min(pos.y, 1.);
        
        float dist = length(pos-uv);
        light *= step(cellcore_radius, dist);
        float cell_width = abs(sin(u_time))*.1;
        
        if((dist) <= cell_width) {
            //cells = vec3(dist+random(pos*sin(.122736378)), random(pos*sin(0.232736)), random(pos*sin(.2328173)));
            cells *= vec3(1.-dist*5.);
        }
    }
    out_color = vec4(col + light * cells, 1.0);
}