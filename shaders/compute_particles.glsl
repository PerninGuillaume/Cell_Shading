#version 450

layout( local_size_x = 1000 ) in;

uniform float Gravity1 = 1000.0;
uniform vec3 BlackHolePos1 = vec3(0,-5,0);

uniform float Gravity2 = 1000.0;
uniform vec3 BlackHolePos2 = vec3(0,-5,0);

uniform float ParticleMass = 0.1;
uniform float ParticleInvMass = 1.0 / 0.1;
uniform float DeltaT = 0.0005;
uniform float MaxDist = 10.0f;

layout(std430, binding=0) buffer Pos {
  vec4 Position[];
};
layout(std430, binding=1) buffer Vel {
  vec4 Velocity[];
};
layout(std430, binding=2) buffer initPos {
  vec4 initPosition[];
};
layout(std430, binding=3) buffer Life {
  vec4 LifeVec[];
};



/////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////// Code taken from stack overflow////////////////////////////////////////
////////// url : https://stackoverflow.com/questions/4200224/random-noise-functions-for-glsl ////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
    static.frag
    by Spatial
    05 July 2013
*/

// A single iteration of Bob Jenkins' One-At-A-Time hashing algorithm.
uint hash( uint x ) {
  x += ( x << 10u );
  x ^= ( x >>  6u );
  x += ( x <<  3u );
  x ^= ( x >> 11u );
  x += ( x << 15u );
  return x;
}



// Compound versions of the hashing algorithm I whipped together.
uint hash( uvec2 v ) { return hash( v.x ^ hash(v.y)                         ); }
uint hash( uvec3 v ) { return hash( v.x ^ hash(v.y) ^ hash(v.z)             ); }
uint hash( uvec4 v ) { return hash( v.x ^ hash(v.y) ^ hash(v.z) ^ hash(v.w) ); }



// Construct a float with half-open range [0:1] using low 23 bits.
// All zeroes yields 0.0, all ones yields the next smallest representable value below 1.0.
float floatConstruct( uint m ) {
  const uint ieeeMantissa = 0x007FFFFFu; // binary32 mantissa bitmask
  const uint ieeeOne      = 0x3F800000u; // 1.0 in IEEE binary32

  m &= ieeeMantissa;                     // Keep only mantissa bits (fractional part)
  m |= ieeeOne;                          // Add fractional part to 1.0

  float  f = uintBitsToFloat( m );       // Range [1:2]
  return f - 1.0;                        // Range [0:1]
}



// Pseudo-random value in half-open range [0:1].
float random( float x ) { return floatConstruct(hash(floatBitsToUint(x))); }
float random( vec2  v ) { return floatConstruct(hash(floatBitsToUint(v))); }
float random( vec3  v ) { return floatConstruct(hash(floatBitsToUint(v))); }
float random( vec4  v ) { return floatConstruct(hash(floatBitsToUint(v))); }

///////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////// End of code taken from stack overflow /////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

void main() {
  uint idx = gl_GlobalInvocationID.x;

  vec3 p = Position[idx].xyz;

  // Force from gravity
  vec3 d = vec3(0, 2, 0);
  float dist = length(initPosition[idx].xyz - p);
  vec3 force = (Gravity1) * normalize(d);

  // Reset particles that get too far
  if( dist + random(vec3(initPosition[idx].xz, 100)) * 0 > LifeVec[idx].x / 4)
  {
    Position[idx] = initPosition[idx] + 0.3 - 0.6 *random(vec3(Position[idx].xyz));
    Velocity[idx].xyz = vec3(0,0,0);
  }
  else {
    // Apply simple Euler integrator
    vec3 a = force * ParticleInvMass;
    Position[idx] = vec4(
    p + Velocity[idx].xyz * DeltaT + 0.5 * a * DeltaT * DeltaT, 1.0);
    Velocity[idx] = vec4( Velocity[idx].xyz + a * DeltaT, 0.0);
  }
}
