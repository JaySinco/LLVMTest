#version 330

uniform vec3 iResolution;
uniform float iTime;
uniform float iTimeDelta;
uniform int iFrame;
uniform vec4 iMouse;
uniform vec4 iDate;
uniform sampler2D iChannel0;

// Ray tracing: the next week, chapter 9: A scene testing all new features. Created by Reinder Nijhoff 2018
// @reindernijhoff
//
// https://www.shadertoy.com/view/MtycDD
//
// These shaders are my implementation of the raytracer described in the (excellent)
// book "Ray tracing in one weekend" and "Ray tracing: the next week"[1] by Peter Shirley
// (@Peter_shirley). I have tried to follow the code from his book as much as possible.
//
// [1] http://in1weekend.blogspot.com/2016/01/ray-tracing-in-one-weekend.html
//

#define MAX_FLOAT 1e5
#define EPSILON 0.01
#define MAX_RECURSION (24+min(0,iFrame))

#define LAMBERTIAN 0
#define METAL 1
#define DIELECTRIC 2
#define DIFFUSE_LIGHT 3
#define ISOTROPIC 4

#define SPHERE 0
#define MOVING_SPHERE 1
#define BOX 2
#define CONSTANT_MEDIUM_SPHERE 3
#define CONSTANT_MEDIUM_BOX 4

#define SOLID 0
#define NOISE 1

//
// Hash functions by Nimitz:
// https://www.shadertoy.com/view/Xt3cDn
//

uint base_hash(uvec2 p) {
    p = 1103515245U * ((p >> 1U) ^ (p.yx));
    uint h32 = 1103515245U * ((p.x) ^ (p.y >> 3U));
    return h32 ^ (h32 >> 16);
}

float g_seed = 0.;

float hash1(inout float seed) {
    uint n = base_hash(floatBitsToUint(vec2(seed += .1, seed += .1)));
    return float(n) / float(0xffffffffU);
}

vec2 hash2(inout float seed) {
    uint n = base_hash(floatBitsToUint(vec2(seed += .1, seed += .1)));
    uvec2 rz = uvec2(n, n * 48271U);
    return vec2(rz.xy & uvec2(0x7fffffffU)) / float(0x7fffffff);
}

vec3 hash3(inout float seed) {
    uint n = base_hash(floatBitsToUint(vec2(seed += .1, seed += .1)));
    uvec3 rz = uvec3(n, n * 16807U, n * 48271U);
    return vec3(rz & uvec3(0x7fffffffU)) / float(0x7fffffff);
}

//
// Noise functions by Inigo Quilez:
// https://www.shadertoy.com/view/4sfGzS
//

float hash(vec3 p) {
    p = fract(p * 0.3183099 + .1);
    p *= 17.0;
    return 2. * fract(p.x * p.y * p.z * (p.x + p.y + p.z)) - 1.;
}

float noise(const in vec3 x) {
    vec3 p = floor(x);
    vec3 f = fract(x);
    f = f * f * (3.0 - 2.0 * f);

    return mix(mix(mix(hash(p + vec3(0, 0, 0)), hash(p + vec3(1, 0, 0)), f.x), mix(hash(p + vec3(0, 1, 0)), hash(p + vec3(1, 1, 0)), f.x), f.y), mix(mix(hash(p + vec3(0, 0, 1)), hash(p + vec3(1, 0, 1)), f.x), mix(hash(p + vec3(0, 1, 1)), hash(p + vec3(1, 1, 1)), f.x), f.y), f.z);
}

float fbm(const in vec3 p, const in int octaves) {
    float accum = 0.;
    vec3 temp_p = p;
    float weight = 1.;

    for(int i = 0; i < octaves; i++) {
        accum += weight * noise(temp_p);
        weight *= .5;
        temp_p *= 2.;
    }
    return abs(accum);
}

//
// Ray trace helper functions
//

float schlick(float cosine, float ior) {
    float r0 = (1. - ior) / (1. + ior);
    r0 = r0 * r0;
    return r0 + (1. - r0) * pow((1. - cosine), 5.);
}

bool modified_refract(const in vec3 v, const in vec3 n, const in float ni_over_nt, out vec3 refracted) {
    float dt = dot(v, n);
    float discriminant = 1. - ni_over_nt * ni_over_nt * (1. - dt * dt);
    if(discriminant > 0.) {
        refracted = ni_over_nt * (v - n * dt) - n * sqrt(discriminant);
        return true;
    } else {
        return false;
    }
}

vec3 random_cos_weighted_hemisphere_direction(const vec3 n, inout float seed) {
    vec2 r = hash2(seed);
    vec3 uu = normalize(cross(n, abs(n.y) > .5 ? vec3(1., 0., 0.) : vec3(0., 1., 0.)));
    vec3 vv = cross(uu, n);
    float ra = sqrt(r.y);
    float rx = ra * cos(6.28318530718 * r.x);
    float ry = ra * sin(6.28318530718 * r.x);
    float rz = sqrt(1. - r.y);
    vec3 rr = vec3(rx * uu + ry * vv + rz * n);
    return normalize(rr);
}

vec2 random_in_unit_disk(inout float seed) {
    vec2 h = hash2(seed) * vec2(1., 6.28318530718);
    float phi = h.y;
    float r = sqrt(h.x);
    return r * vec2(sin(phi), cos(phi));
}

vec3 random_in_unit_sphere(inout float seed) {
    vec3 h = hash3(seed) * vec3(2., 6.28318530718, 1.) - vec3(1, 0, 0);
    float phi = h.y;
    float r = pow(h.z, 1. / 3.);
    return r * vec3(sqrt(1. - h.x * h.x) * vec2(sin(phi), cos(phi)), h.x);
}

vec3 rotate_y(const in vec3 p, const in float t) {
    float co = cos(t);
    float si = sin(t);
    vec2 xz = mat2(co, si, -si, co) * p.xz;
    return vec3(xz.x, p.y, xz.y);
}

//
// Ray
//

struct ray {
    vec3 origin, direction;
    float time;
};

ray ray_translate(const in ray r, const in vec3 t) {
    ray rt = r;
    rt.origin -= t;
    return rt;
}

ray ray_rotate_y(const in ray r, const in float t) {
    ray rt = r;
    rt.origin = rotate_y(rt.origin, t);
    rt.direction = rotate_y(rt.direction, t);
    return rt;
}

//
// Texture
//

struct texture_ {
    int type;
    vec3 v;
};

vec3 texture_value(const in texture_ t, const in vec3 p) {
    if(t.type == SOLID) {
        return t.v;
    } else if(t.type == NOISE) {
        return vec3(.5 * (1. + sin(t.v.x * p.x + 5. * fbm((t.v.x) * p, 7))));
    }
}

#define NO_TEX texture_(SOLID,vec3(0))

//
// Material
//

struct material {
    int type;
    texture_ albedo;
    texture_ emit;
    float v;
};

//
// Hit record
//

struct hit_record {
    float t;
    vec3 p, normal;
    material mat;
};

hit_record hit_record_translate(const in hit_record h, const in vec3 t) {
    hit_record ht = h;
    ht.p -= t;
    return ht;
}

hit_record hit_record_rotate_y(const in hit_record h, const in float t) {
    hit_record ht = h;
    ht.p = rotate_y(ht.p, t);
    ht.normal = rotate_y(ht.normal, t);
    return ht;
}

bool material_scatter(const in ray r_in, const in hit_record rec, out vec3 attenuation, out ray scattered) {
    if(rec.mat.type == LAMBERTIAN) {
        scattered = ray(rec.p, random_cos_weighted_hemisphere_direction(rec.normal, g_seed), r_in.time);
        attenuation = texture_value(rec.mat.albedo, rec.p);
        return true;
    } else if(rec.mat.type == METAL) {
        vec3 rd = reflect(r_in.direction, rec.normal);
        scattered = ray(rec.p, normalize(rd + rec.mat.v * random_in_unit_sphere(g_seed)), r_in.time);
        attenuation = texture_value(rec.mat.albedo, rec.p);
        return true;
    } else if(rec.mat.type == DIELECTRIC) {
        vec3 outward_normal, refracted, reflected = reflect(r_in.direction, rec.normal);
        float ni_over_nt, reflect_prob, cosine;

        attenuation = vec3(1);
        if(dot(r_in.direction, rec.normal) > 0.) {
            outward_normal = -rec.normal;
            ni_over_nt = rec.mat.v;
            cosine = dot(r_in.direction, rec.normal);
            cosine = sqrt(1. - rec.mat.v * rec.mat.v * (1. - cosine * cosine));
        } else {
            outward_normal = rec.normal;
            ni_over_nt = 1. / rec.mat.v;
            cosine = -dot(r_in.direction, rec.normal);
        }

        if(modified_refract(r_in.direction, outward_normal, ni_over_nt, refracted)) {
            reflect_prob = schlick(cosine, rec.mat.v);
        } else {
            reflect_prob = 1.;
        }

        if(hash1(g_seed) < reflect_prob) {
            scattered = ray(rec.p, reflected, r_in.time);
        } else {
            scattered = ray(rec.p, refracted, r_in.time);
        }
        return true;
    } else if(rec.mat.type == ISOTROPIC) {
        scattered = ray(rec.p, random_in_unit_sphere(g_seed), r_in.time);
        attenuation = texture_value(rec.mat.albedo, rec.p);
        return true;
    }

    return false;
}

vec3 material_emitted(const in hit_record rec) {
    if(rec.mat.type == DIFFUSE_LIGHT) {
        return texture_value(rec.mat.emit, rec.p);
    } else {
        return vec3(0);
    }
}

//
// Hitable
//

struct hitable {
    int type;
    vec3 center, v3; // v3 is speed for moving sphere (with center at t=0)
                     //    or dimensions for box.
    float v;         // Radius for sphere.
};

bool sphere_intersect(const in ray r, const in float t_min, const in float t_max, const in vec3 center, const in float radius, inout float dist) {
    vec3 oc = r.origin - center;
    float b = dot(oc, r.direction);
    float c = dot(oc, oc) - radius * radius;
    float discriminant = b * b - c;
    if(discriminant < 0.0)
        return false;

    float s = sqrt(discriminant);
    float t1 = -b - s;
    float t2 = -b + s;

    float t = t1 < t_min ? t2 : t1;
    if(t < t_max && t > t_min) {
        dist = t;
        return true;
    } else {
        return false;
    }
}

bool box_intersect(const in ray r, const in float t_min, const in float t_max, const in vec3 center, const in vec3 rad, out vec3 normal, inout float dist) {
    vec3 m = 1. / r.direction;
    vec3 n = m * (r.origin - center);
    vec3 k = abs(m) * rad;

    vec3 t1 = -n - k;
    vec3 t2 = -n + k;

    float tN = max(max(t1.x, t1.y), t1.z);
    float tF = min(min(t2.x, t2.y), t2.z);

    if(tN > tF || tF < 0.)
        return false;

    float t = tN < t_min ? tF : tN;
    if(t < t_max && t > t_min) {
        dist = t;
        normal = -sign(r.direction) * step(t1.yzx, t1.xyz) * step(t1.zxy, t1.xyz);
        return true;
    } else {
        return false;
    }
}

bool hitable_hit(const in hitable hb, const in ray r, const in float t_min, const in float t_max, inout hit_record rec) {

    if(hb.type == SPHERE || hb.type == MOVING_SPHERE) {
        vec3 center = hb.type == SPHERE ? hb.center : hb.center + r.time * hb.v3;
        float radius = hb.v;
        float dist;
        if(sphere_intersect(r, t_min, t_max, center, radius, dist)) {
            rec.t = dist;
            rec.p = r.origin + dist * r.direction;
            rec.normal = (rec.p - center) / radius;
            return true;
        } else {
            return false;
        }
    } else if(hb.type == BOX) {
        float dist;
        vec3 normal;
        if(box_intersect(r, t_min, t_max, hb.center, hb.v3, normal, dist)) {
            rec.t = dist;
            rec.p = r.origin + dist * r.direction;
            rec.normal = normal;
            return true;
        } else {
            return false;
        }
    } else { // constant medium
        bool h1, h2;
        float t1, t2;
        hit_record rec1, rec2;
        if(hb.type == CONSTANT_MEDIUM_SPHERE) {
            h1 = sphere_intersect(r, -MAX_FLOAT, MAX_FLOAT, hb.center, hb.v3.x, t1);
            h2 = sphere_intersect(r, t1 + EPSILON, MAX_FLOAT, hb.center, hb.v3.x, t2);
        } else { // box
            vec3 normal;
            h1 = box_intersect(r, -MAX_FLOAT, MAX_FLOAT, hb.center, hb.v3, normal, t1);
            h2 = box_intersect(r, t1 + EPSILON, MAX_FLOAT, hb.center, hb.v3, normal, t2);
        }
        if(h1 && h2) {
            if(t1 < t_min)
                t1 = t_min;
            if(t2 > t_max)
                t2 = t_max;
            if(t1 >= t2) {
                return false;
            } else {
                if(t1 < 0.)
                    t1 = 0.;

                float distance_inside_boundary = t2 - t1;
                float hit_distance = -(1. / hb.v) * log(hash1(g_seed));

                if(hit_distance < distance_inside_boundary) {
                    rec.t = t1 + hit_distance;
                    rec.p = r.origin + r.direction * rec.t;
                    rec.normal = vec3(1, 0, 0);  // arbitrary
                    return true;
                } else {
                    return false;
                }
            }
        } else {
            return false;
        }
    }
}

//
// Camera
//

struct camera {
    vec3 origin, lower_left_corner, horizontal, vertical, u, v, w;
    float time0, time1, lens_radius;
};

camera camera_const(const in vec3 lookfrom, const in vec3 lookat, const in vec3 vup, const in float vfov, const in float aspect, const in float aperture, const in float focus_dist, const in float time0, const in float time1) {
    camera cam;
    cam.lens_radius = aperture / 2.;
    float theta = vfov * 3.14159265359 / 180.;
    float half_height = tan(theta / 2.);
    float half_width = aspect * half_height;
    cam.origin = lookfrom;
    cam.w = normalize(lookfrom - lookat);
    cam.u = normalize(cross(vup, cam.w));
    cam.v = cross(cam.w, cam.u);
    cam.lower_left_corner = cam.origin - half_width * focus_dist * cam.u - half_height * focus_dist * cam.v - focus_dist * cam.w;
    cam.horizontal = 2. * half_width * focus_dist * cam.u;
    cam.vertical = 2. * half_height * focus_dist * cam.v;
    cam.time0 = time0;
    cam.time1 = time1;
    return cam;
}

ray camera_get_ray(camera c, vec2 uv) {
    vec2 rd = c.lens_radius * random_in_unit_disk(g_seed);
    vec3 offset = c.u * rd.x + c.v * rd.y;
    return ray(c.origin + offset, normalize(c.lower_left_corner + uv.x * c.horizontal + uv.y * c.vertical - c.origin - offset), mix(c.time0, c.time1, hash1(g_seed)));
}

//
// Color & Scene
//

bool world_hit(const in ray r, const in float t_min, const in float t_max, out hit_record rec) {
    rec.t = t_max;
    bool hit = false;

    if(hitable_hit(hitable(BOX, vec3(273, 555, 279.5), vec3(150, .1, 132.5), 0.), r, t_min, rec.t, rec))
        hit = true, rec.mat = material(DIFFUSE_LIGHT, NO_TEX, texture_(SOLID, vec3(7)), 0.);

    if(hitable_hit(hitable(MOVING_SPHERE, vec3(400, 400, 200), vec3(30, 0, 0), 50.), r, t_min, rec.t, rec))
        hit = true, rec.mat = material(LAMBERTIAN, texture_(SOLID, vec3(.7, .3, .1)), NO_TEX, 0.);
    if(hitable_hit(hitable(SPHERE, vec3(260, 150, 45), vec3(0), 50.), r, t_min, rec.t, rec))
        hit = true, rec.mat = material(DIELECTRIC, NO_TEX, NO_TEX, 1.5);
    if(hitable_hit(hitable(SPHERE, vec3(0, 150, 145), vec3(0), 50.), r, t_min, rec.t, rec))
        hit = true, rec.mat = material(METAL, texture_(SOLID, vec3(.8, .8, .9)), NO_TEX, 1.);
    if(hitable_hit(hitable(SPHERE, vec3(220, 280, 300), vec3(0), 80.), r, t_min, rec.t, rec))
        hit = true, rec.mat = material(LAMBERTIAN, texture_(NOISE, vec3(.1)), NO_TEX, .1);

    if(hitable_hit(hitable(SPHERE, vec3(360, 150, 145), vec3(0), 70.), r, t_min, rec.t, rec))
        hit = true, rec.mat = material(DIELECTRIC, NO_TEX, NO_TEX, 1.5);
    if(hitable_hit(hitable(CONSTANT_MEDIUM_SPHERE, vec3(360, 150, 145), vec3(70), .2), r, t_min, rec.t, rec))
        hit = true, rec.mat = material(ISOTROPIC, texture_(SOLID, vec3(.2, .4, .9)), NO_TEX, 0.);

    if(hitable_hit(hitable(SPHERE, vec3(400, 200, 400), vec3(0), 100.), r, t_min, rec.t, rec))
        hit = true, rec.mat = material(DIELECTRIC, NO_TEX, NO_TEX, 1.04);
    if(hitable_hit(hitable(CONSTANT_MEDIUM_SPHERE, vec3(400, 200, 400), vec3(100), .1), r, t_min, rec.t, rec))
        hit = true, rec.mat = material(ISOTROPIC, texture_(SOLID, vec3(.4, .5, .7)), NO_TEX, 0.);

    if(hitable_hit(hitable(CONSTANT_MEDIUM_SPHERE, vec3(0), vec3(1000), .0001), r, t_min, rec.t, rec))
        hit = true, rec.mat = material(ISOTROPIC, texture_(SOLID, vec3(1)), NO_TEX, 0.);

// A 2D grid for the ground and a 3D grid for the cube with spheres are implemented as an optimization

// cube with spheres
    ray r_ = ray_rotate_y(ray_translate(r, vec3(-100, 270, 395) + 82.5), 15. / 180. * 3.14159265359);
    vec3 normal;
    float dist;
    if(box_intersect(r_, EPSILON, MAX_FLOAT, vec3(0), vec3(100), normal, dist)) {
        const float cubeGridScale = 20.;
        vec3 ro = r_.origin / cubeGridScale;
        vec3 pos = floor(ro + (all(lessThan(abs(r_.origin), vec3(100))) ? EPSILON : dist) * r_.direction / cubeGridScale);
        vec3 rdi = 1. / r_.direction;
        vec3 rda = abs(rdi);
        vec3 rds = sign(r.direction);
        vec3 dis = (pos - ro + .5 + rds * .5) * rdi;
        bool b_hit = false;

        // traverse grid in 3D
        vec3 mm = vec3(0);
        int steps = 12 + min(0, iFrame);
        for(int i = 0; i < steps; i++) {
            for(float x = -1.; x <= 1.; x += 1.) {
                for(float y = -1.; y <= 1.; y += 1.) {
                    for(float z = -1.; z <= 1.; z += 1.) {
                        vec3 posc = pos + vec3(x, y, z);
                        if(all(lessThan(abs(posc), vec3(4.1)))) {
                            float seed = dot(posc, vec3(.1, .11, .111));
                            vec3 scenter = posc * cubeGridScale + cubeGridScale * ((hash3(seed) - .5) * .75);
                            hit_record rec_ = rec;
                            if(hitable_hit(hitable(SPHERE, scenter, vec3(0), 10.), r_, t_min, rec_.t, rec_))
                                b_hit = true, rec = hit_record_translate(hit_record_rotate_y(rec_, -15. / 180. * 3.14159265359), -82.5 - vec3(-100, 270, 395)), rec.mat = material(LAMBERTIAN, texture_(SOLID, vec3(.73)), NO_TEX, 0.);
                        }
                    }
                }
            }

            if(b_hit) {
                hit = true;
                break;
            }

            // step to next cell
            vec3 mm = step(dis.xyz, dis.yxy) * step(dis.xyz, dis.zzx);
            dis += mm * rda;
            pos += mm * rds;
        }
    }

// floor
    if(r.origin.y < 101. || r.direction.y < 0.) {
        const float floorGridScale = 100.;
        vec3 ro = r.origin / floorGridScale;
        vec2 pos = floor(ro.xz);
        vec3 rdi = 1. / r.direction;
        vec3 rda = abs(rdi);
        vec2 rds = sign(r.direction.xz);
        vec2 dis = (pos - ro.xz + .5 + rds * .5) * rdi.xz;
        bool b_hit = false;

        // traverse grid in 2D
        vec2 mm = vec2(0);
        int steps = 26 + min(0, iFrame);
        for(int i = 0; i < steps; i++) {
            float seed = dot(pos, vec2(.7, .17));
            vec3 bcenter = vec3(pos.x * floorGridScale + floorGridScale * .5, 0, pos.y * floorGridScale + .5 * floorGridScale);
            vec3 bsize = vec3(floorGridScale * .5, 100. * (hash1(seed) + 0.01), floorGridScale * .5);

            if(hitable_hit(hitable(BOX, bcenter, bsize, 0.), r, t_min, rec.t, rec))
                b_hit = true, rec.mat = material(LAMBERTIAN, texture_(SOLID, vec3(.48, .83, .53)), NO_TEX, 0.);

            if(b_hit) {
                hit = true;
                break;
            }

            // step to next cell
            mm = step(dis.xy, dis.yx);
            dis += mm * rda.xz;
            pos += mm * rds;
        }
    }

    return hit;
}

vec3 color(in ray r) {
    vec3 col = vec3(0);
    vec3 emitted = vec3(0);
    hit_record rec;

    for(int i = 0; i < MAX_RECURSION; i++) {
        if(world_hit(r, EPSILON, MAX_FLOAT, rec)) {
            ray scattered;
            vec3 attenuation;
            vec3 emit = material_emitted(rec);
            emitted += i == 0 ? emit : col * emit;

            if(material_scatter(r, rec, attenuation, scattered)) {
                col = i == 0 ? attenuation : col * attenuation;
                r = scattered;
            } else {
                return emitted;
            }
        } else {
            return emitted;
        }
        if(dot(col, col) < 0.0001)
            return emitted; // optimisation
    }
    return emitted;
}

//
// Main
//

void mainImage(out vec4 frag_color, in vec2 frag_coord) {
    if(ivec2(frag_coord) == ivec2(0)) {
        frag_color = iResolution.xyxy;
    } else {
        g_seed = float(base_hash(floatBitsToUint(frag_coord))) / float(0xffffffffU) + iTime;

        vec2 uv = (frag_coord + hash2(g_seed)) / iResolution.xy;
        float aspect = iResolution.x / iResolution.y;
        vec3 lookfrom = vec3(478, 278, -600);
        vec3 lookat = vec3(278, 278, 0);

        camera cam = camera_const(lookfrom, lookat, vec3(0, 1, 0), 40., aspect, 2.0, 800., 0., 1.);
        ray r = camera_get_ray(cam, uv);
        vec3 col = color(r);

        // if(texelFetch(iChannel0, ivec2(0), 0).xy == iResolution.xy) {
        //     frag_color = vec4(col, 1) + texelFetch(iChannel0, ivec2(frag_coord), 0);
        // } else {
        //     frag_color = vec4(col, 1);
        // }

        if(iFrame >= 1) {
            vec3 old = texelFetch(iChannel0, ivec2(frag_coord), 0).rgb;
            frag_color = vec4((old * (iFrame - 1) + col) / iFrame, 1.0);
        } else {
            frag_color = vec4(col, 1.0);
        }
    }
}

void main() {
    mainImage(gl_FragColor, gl_FragCoord.xy);
}