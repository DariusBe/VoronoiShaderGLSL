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
    vec3 light = vec3(1.);
    float cellcore_radius = 0.01;

    vec3 col = vec3(0);
    const int cell_count = 15;
    vec2 array[cell_count];
    vec3 cells = vec3(1);



    for(int i=0; i<cell_count; i++) {

        vec2 pos = vec2(
            random(vec2(i+1)),
            random(vec2(i+1)*0.5*cos(float(i)*.2387))
        );

        

        pos = getPointAtCursor(pos, mouse); //Setzt (bei gehaltener Maus) Voronoi-Zelle an Cursor 
        

        pos.x = min(pos.x, 1.);
        pos.y = min(pos.y, 1.);
        
        array[i] = pos;

        float dist = length(pos-uv);
        if(dist <= cellcore_radius) {
            light *= vec3(1,0,0);
        }
        float cell_width = .11;
        
        if((dist) <= cell_width) {
            //cells = vec3(dist+random(pos*sin(.122736378)), random(pos*sin(0.232736)), random(pos*sin(.2328173)));
           cells *= vec3(1.-dist*5.);
        }

    }

    float diff = 0.;

    for(int i=0; i<cell_count; i++) {
        for(int j=0; j<cell_count; j++) {
            diff = length((array[i]-uv)-(array[j]-uv)) / 2.;
        }
    }

    out_color = vec4(col + light * cells, 1.0);
}