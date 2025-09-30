struct AtmosphereParams{
    float4 groundAlbedo; //xyz - colour, w: Radius bottom
    float4 rayleighScattering; //xyz - wavelenght dependad, w :radius top
    float4 mieScattering;  //xyz: as-is, w:Reayleigh density exp scale 
    float4 mieAbsorption;//xyz: as-is, w: Mie density exp scale
    float4 mieExctinction;//xyz: as-is, w: Mie phase G
    float4 absorptionExctinction; //xyz: as-is, w: Absorption denssity 0 layer width
}