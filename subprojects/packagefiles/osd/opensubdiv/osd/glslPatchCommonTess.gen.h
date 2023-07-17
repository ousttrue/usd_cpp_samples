"//\n"
"//   Copyright 2013-2018 Pixar\n"
"//\n"
"//   Licensed under the Apache License, Version 2.0 (the \"Apache License\")\n"
"//   with the following modification; you may not use this file except in\n"
"//   compliance with the Apache License and the following modification to it:\n"
"//   Section 6. Trademarks. is deleted and replaced with:\n"
"//\n"
"//   6. Trademarks. This License does not grant permission to use the trade\n"
"//      names, trademarks, service marks, or product names of the Licensor\n"
"//      and its affiliates, except as required to comply with Section 4(c) of\n"
"//      the License and to reproduce the content of the NOTICE file.\n"
"//\n"
"//   You may obtain a copy of the Apache License at\n"
"//\n"
"//       http://www.apache.org/licenses/LICENSE-2.0\n"
"//\n"
"//   Unless required by applicable law or agreed to in writing, software\n"
"//   distributed under the Apache License with the above modification is\n"
"//   distributed on an \"AS IS\" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY\n"
"//   KIND, either express or implied. See the Apache License for the specific\n"
"//   language governing permissions and limitations under the Apache License.\n"
"//\n"
"\n"
"// ----------------------------------------------------------------------------\n"
"// Tessellation\n"
"// ----------------------------------------------------------------------------\n"
"\n"
"// For now, fractional spacing is supported only with screen space tessellation\n"
"#ifndef OSD_ENABLE_SCREENSPACE_TESSELLATION\n"
"#undef OSD_FRACTIONAL_EVEN_SPACING\n"
"#undef OSD_FRACTIONAL_ODD_SPACING\n"
"#endif\n"
"\n"
"#if defined OSD_FRACTIONAL_EVEN_SPACING\n"
"  #define OSD_SPACING fractional_even_spacing\n"
"#elif defined OSD_FRACTIONAL_ODD_SPACING\n"
"  #define OSD_SPACING fractional_odd_spacing\n"
"#else\n"
"  #define OSD_SPACING equal_spacing\n"
"#endif\n"
"\n"
"//\n"
"// Organization of B-spline and Bezier control points.\n"
"//\n"
"// Each patch is defined by 16 control points (labeled 0-15).\n"
"//\n"
"// The patch will be evaluated across the domain from (0,0) at\n"
"// the lower-left to (1,1) at the upper-right. When computing\n"
"// adaptive tessellation metrics, we consider refined vertex-vertex\n"
"// and edge-vertex points along the transition edges of the patch\n"
"// (labeled vv* and ev* respectively).\n"
"//\n"
"// The two segments of each transition edge are labeled Lo and Hi,\n"
"// with the Lo segment occurring before the Hi segment along the\n"
"// transition edge's domain parameterization. These Lo and Hi segment\n"
"// tessellation levels determine how domain evaluation coordinates\n"
"// are remapped along transition edges. The Hi segment value will\n"
"// be zero for a non-transition edge.\n"
"//\n"
"// (0,1)                                         (1,1)\n"
"//\n"
"//   vv3                  ev23                   vv2\n"
"//        |       Lo3       |       Hi3       |\n"
"//      --O-----------O-----+-----O-----------O--\n"
"//        | 12        | 13     14 |        15 |\n"
"//        |           |           |           |\n"
"//        |           |           |           |\n"
"//    Hi0 |           |           |           | Hi2\n"
"//        |           |           |           |\n"
"//        O-----------O-----------O-----------O\n"
"//        | 8         | 9      10 |        11 |\n"
"//        |           |           |           |\n"
"// ev03 --+           |           |           +-- ev12\n"
"//        |           |           |           |\n"
"//        | 4         | 5       6 |         7 |\n"
"//        O-----------O-----------O-----------O\n"
"//        |           |           |           |\n"
"//    Lo0 |           |           |           | Lo2\n"
"//        |           |           |           |\n"
"//        |           |           |           |\n"
"//        | 0         | 1       2 |         3 |\n"
"//      --O-----------O-----+-----O-----------O--\n"
"//        |       Lo1       |       Hi1       |\n"
"//   vv0                  ev01                   vv1\n"
"//\n"
"// (0,0)                                         (1,0)\n"
"//\n"
"\n"
"#define OSD_MAX_TESS_LEVEL gl_MaxTessGenLevel\n"
"\n"
"float OsdComputePostProjectionSphereExtent(vec3 center, float diameter)\n"
"{\n"
"    vec4 p = OsdProjectionMatrix() * vec4(center, 1.0);\n"
"    return abs(diameter * OsdProjectionMatrix()[1][1] / p.w);\n"
"}\n"
"\n"
"float OsdComputeTessLevel(vec3 p0, vec3 p1)\n"
"{\n"
"    // Adaptive factor can be any computation that depends only on arg values.\n"
"    // Project the diameter of the edge's bounding sphere instead of using the\n"
"    // length of the projected edge itself to avoid problems near silhouettes.\n"
"    p0 = (OsdModelViewMatrix() * vec4(p0, 1.0)).xyz;\n"
"    p1 = (OsdModelViewMatrix() * vec4(p1, 1.0)).xyz;\n"
"    vec3 center = (p0 + p1) / 2.0;\n"
"    float diameter = distance(p0, p1);\n"
"    float projLength = OsdComputePostProjectionSphereExtent(center, diameter);\n"
"    float tessLevel = max(1.0, OsdTessLevel() * projLength);\n"
"\n"
"    // We restrict adaptive tessellation levels to half of the device\n"
"    // supported maximum because transition edges are split into two\n"
"    // halves and the sum of the two corresponding levels must not exceed\n"
"    // the device maximum. We impose this limit even for non-transition\n"
"    // edges because a non-transition edge must be able to match up with\n"
"    // one half of the transition edge of an adjacent transition patch.\n"
"    return min(tessLevel, OSD_MAX_TESS_LEVEL / 2);\n"
"}\n"
"\n"
"void\n"
"OsdGetTessLevelsUniform(ivec3 patchParam,\n"
"                        out vec4 tessOuterLo, out vec4 tessOuterHi)\n"
"{\n"
"    // Uniform factors are simple powers of two for each level.\n"
"    // The maximum here can be increased if we know the maximum\n"
"    // refinement level of the mesh:\n"
"    //     min(OSD_MAX_TESS_LEVEL, pow(2, MaximumRefinementLevel-1)\n"
"    int refinementLevel = OsdGetPatchRefinementLevel(patchParam);\n"
"    float tessLevel = min(OsdTessLevel(), OSD_MAX_TESS_LEVEL) /\n"
"                        pow(2, refinementLevel-1);\n"
"\n"
"    // tessLevels of transition edge should be clamped to 2.\n"
"    int transitionMask = OsdGetPatchTransitionMask(patchParam);\n"
"    vec4 tessLevelMin = vec4(1) + vec4(((transitionMask & 8) >> 3),\n"
"                                       ((transitionMask & 1) >> 0),\n"
"                                       ((transitionMask & 2) >> 1),\n"
"                                       ((transitionMask & 4) >> 2));\n"
"\n"
"    tessOuterLo = max(vec4(tessLevel), tessLevelMin);\n"
"    tessOuterHi = vec4(0);\n"
"}\n"
"\n"
"void\n"
"OsdGetTessLevelsUniformTriangle(ivec3 patchParam,\n"
"                        out vec4 tessOuterLo, out vec4 tessOuterHi)\n"
"{\n"
"    // Uniform factors are simple powers of two for each level.\n"
"    // The maximum here can be increased if we know the maximum\n"
"    // refinement level of the mesh:\n"
"    //     min(OSD_MAX_TESS_LEVEL, pow(2, MaximumRefinementLevel-1)\n"
"    int refinementLevel = OsdGetPatchRefinementLevel(patchParam);\n"
"    float tessLevel = min(OsdTessLevel(), OSD_MAX_TESS_LEVEL) /\n"
"                        pow(2, refinementLevel-1);\n"
"\n"
"    // tessLevels of transition edge should be clamped to 2.\n"
"    int transitionMask = OsdGetPatchTransitionMask(patchParam);\n"
"    vec4 tessLevelMin = vec4(1) + vec4(((transitionMask & 4) >> 2),\n"
"                                       ((transitionMask & 1) >> 0),\n"
"                                       ((transitionMask & 2) >> 1),\n"
"                                       0);\n"
"\n"
"    tessOuterLo = max(vec4(tessLevel), tessLevelMin);\n"
"    tessOuterHi = vec4(0);\n"
"}\n"
"\n"
"void\n"
"OsdGetTessLevelsRefinedPoints(vec3 cp[16], ivec3 patchParam,\n"
"                              out vec4 tessOuterLo, out vec4 tessOuterHi)\n"
"{\n"
"    // Each edge of a transition patch is adjacent to one or two patches\n"
"    // at the next refined level of subdivision. We compute the corresponding\n"
"    // vertex-vertex and edge-vertex refined points along the edges of the\n"
"    // patch using Catmull-Clark subdivision stencil weights.\n"
"    // For simplicity, we let the optimizer discard unused computation.\n"
"\n"
"    vec3 vv0 = (cp[0] + cp[2] + cp[8] + cp[10]) * 0.015625 +\n"
"               (cp[1] + cp[4] + cp[6] + cp[9]) * 0.09375 + cp[5] * 0.5625;\n"
"    vec3 ev01 = (cp[1] + cp[2] + cp[9] + cp[10]) * 0.0625 +\n"
"                (cp[5] + cp[6]) * 0.375;\n"
"\n"
"    vec3 vv1 = (cp[1] + cp[3] + cp[9] + cp[11]) * 0.015625 +\n"
"               (cp[2] + cp[5] + cp[7] + cp[10]) * 0.09375 + cp[6] * 0.5625;\n"
"    vec3 ev12 = (cp[5] + cp[7] + cp[9] + cp[11]) * 0.0625 +\n"
"                (cp[6] + cp[10]) * 0.375;\n"
"\n"
"    vec3 vv2 = (cp[5] + cp[7] + cp[13] + cp[15]) * 0.015625 +\n"
"               (cp[6] + cp[9] + cp[11] + cp[14]) * 0.09375 + cp[10] * 0.5625;\n"
"    vec3 ev23 = (cp[5] + cp[6] + cp[13] + cp[14]) * 0.0625 +\n"
"                (cp[9] + cp[10]) * 0.375;\n"
"\n"
"    vec3 vv3 = (cp[4] + cp[6] + cp[12] + cp[14]) * 0.015625 +\n"
"               (cp[5] + cp[8] + cp[10] + cp[13]) * 0.09375 + cp[9] * 0.5625;\n"
"    vec3 ev03 = (cp[4] + cp[6] + cp[8] + cp[10]) * 0.0625 +\n"
"                (cp[5] + cp[9]) * 0.375;\n"
"\n"
"    tessOuterLo = vec4(0);\n"
"    tessOuterHi = vec4(0);\n"
"\n"
"    int transitionMask = OsdGetPatchTransitionMask(patchParam);\n"
"\n"
"    if ((transitionMask & 8) != 0) {\n"
"        tessOuterLo[0] = OsdComputeTessLevel(vv0, ev03);\n"
"        tessOuterHi[0] = OsdComputeTessLevel(vv3, ev03);\n"
"    } else {\n"
"        tessOuterLo[0] = OsdComputeTessLevel(cp[5], cp[9]);\n"
"    }\n"
"    if ((transitionMask & 1) != 0) {\n"
"        tessOuterLo[1] = OsdComputeTessLevel(vv0, ev01);\n"
"        tessOuterHi[1] = OsdComputeTessLevel(vv1, ev01);\n"
"    } else {\n"
"        tessOuterLo[1] = OsdComputeTessLevel(cp[5], cp[6]);\n"
"    }\n"
"    if ((transitionMask & 2) != 0) {\n"
"        tessOuterLo[2] = OsdComputeTessLevel(vv1, ev12);\n"
"        tessOuterHi[2] = OsdComputeTessLevel(vv2, ev12);\n"
"    } else {\n"
"        tessOuterLo[2] = OsdComputeTessLevel(cp[6], cp[10]);\n"
"    }\n"
"    if ((transitionMask & 4) != 0) {\n"
"        tessOuterLo[3] = OsdComputeTessLevel(vv3, ev23);\n"
"        tessOuterHi[3] = OsdComputeTessLevel(vv2, ev23);\n"
"    } else {\n"
"        tessOuterLo[3] = OsdComputeTessLevel(cp[9], cp[10]);\n"
"    }\n"
"}\n"
"\n"
"//\n"
"//  Patch boundary corners are ordered counter-clockwise from the first\n"
"//  corner while patch boundary edges and their midpoints are similarly\n"
"//  ordered counter-clockwise beginning at the edge preceding corner[0].\n"
"//\n"
"void\n"
"Osd_GetTessLevelsFromPatchBoundaries4(vec3 corners[4], vec3 midpoints[4],\n"
"                 ivec3 patchParam, out vec4 tessOuterLo, out vec4 tessOuterHi)\n"
"{\n"
"    tessOuterLo = vec4(0);\n"
"    tessOuterHi = vec4(0);\n"
"\n"
"    int transitionMask = OsdGetPatchTransitionMask(patchParam);\n"
"\n"
"    if ((transitionMask & 8) != 0) {\n"
"        tessOuterLo[0] = OsdComputeTessLevel(corners[0], midpoints[0]);\n"
"        tessOuterHi[0] = OsdComputeTessLevel(corners[3], midpoints[0]);\n"
"    } else {\n"
"        tessOuterLo[0] = OsdComputeTessLevel(corners[0], corners[3]);\n"
"    }\n"
"    if ((transitionMask & 1) != 0) {\n"
"        tessOuterLo[1] = OsdComputeTessLevel(corners[0], midpoints[1]);\n"
"        tessOuterHi[1] = OsdComputeTessLevel(corners[1], midpoints[1]);\n"
"    } else {\n"
"        tessOuterLo[1] = OsdComputeTessLevel(corners[0], corners[1]);\n"
"    }\n"
"    if ((transitionMask & 2) != 0) {\n"
"        tessOuterLo[2] = OsdComputeTessLevel(corners[1], midpoints[2]);\n"
"        tessOuterHi[2] = OsdComputeTessLevel(corners[2], midpoints[2]);\n"
"    } else {\n"
"        tessOuterLo[2] = OsdComputeTessLevel(corners[1], corners[2]);\n"
"    }\n"
"    if ((transitionMask & 4) != 0) {\n"
"        tessOuterLo[3] = OsdComputeTessLevel(corners[3], midpoints[3]);\n"
"        tessOuterHi[3] = OsdComputeTessLevel(corners[2], midpoints[3]);\n"
"    } else {\n"
"        tessOuterLo[3] = OsdComputeTessLevel(corners[3], corners[2]);\n"
"    }\n"
"}\n"
"\n"
"void\n"
"Osd_GetTessLevelsFromPatchBoundaries3(vec3 corners[3], vec3 midpoints[3],\n"
"                 ivec3 patchParam, out vec4 tessOuterLo, out vec4 tessOuterHi)\n"
"{\n"
"    tessOuterLo = vec4(0);\n"
"    tessOuterHi = vec4(0);\n"
"\n"
"    int transitionMask = OsdGetPatchTransitionMask(patchParam);\n"
"\n"
"    if ((transitionMask & 4) != 0) {\n"
"        tessOuterLo[0] = OsdComputeTessLevel(corners[0], midpoints[0]);\n"
"        tessOuterHi[0] = OsdComputeTessLevel(corners[2], midpoints[0]);\n"
"    } else {\n"
"        tessOuterLo[0] = OsdComputeTessLevel(corners[0], corners[2]);\n"
"    }\n"
"    if ((transitionMask & 1) != 0) {\n"
"        tessOuterLo[1] = OsdComputeTessLevel(corners[0], midpoints[1]);\n"
"        tessOuterHi[1] = OsdComputeTessLevel(corners[1], midpoints[1]);\n"
"    } else {\n"
"        tessOuterLo[1] = OsdComputeTessLevel(corners[0], corners[1]);\n"
"    }\n"
"    if ((transitionMask & 2) != 0) {\n"
"        tessOuterLo[2] = OsdComputeTessLevel(corners[2], midpoints[2]);\n"
"        tessOuterHi[2] = OsdComputeTessLevel(corners[1], midpoints[2]);\n"
"    } else {\n"
"        tessOuterLo[2] = OsdComputeTessLevel(corners[1], corners[2]);\n"
"    }\n"
"}\n"
"\n"
"vec3\n"
"Osd_EvalBezierCurveMidPoint(vec3 p0, vec3 p1, vec3 p2, vec3 p3)\n"
"{\n"
"    //  Coefficients for the midpoint are { 1/8, 3/8, 3/8, 1/8 }:\n"
"    return 0.125 * (p0 + p3) + 0.375 * (p1 + p2);\n"
"}\n"
"\n"
"vec3\n"
"Osd_EvalQuarticBezierCurveMidPoint(vec3 p0, vec3 p1, vec3 p2, vec3 p3, vec3 p4)\n"
"{\n"
"    //  Coefficients for the midpoint are { 1/16, 1/4, 3/8, 1/4, 1/16 }:\n"
"    return 0.0625 * (p0 + p4) + 0.25 * (p1 + p3) + 0.375 * p2;\n"
"}\n"
"\n"
"void\n"
"OsdEvalPatchBezierTessLevels(OsdPerPatchVertexBezier cpBezier[16],\n"
"                 ivec3 patchParam, out vec4 tessOuterLo, out vec4 tessOuterHi)\n"
"{\n"
"    // Each edge of a transition patch is adjacent to one or two patches\n"
"    // at the next refined level of subdivision. When the patch control\n"
"    // points have been converted to the Bezier basis, the control points\n"
"    // at the four corners are on the limit surface (since a Bezier patch\n"
"    // interpolates its corner control points). We can compute an adaptive\n"
"    // tessellation level for transition edges on the limit surface by\n"
"    // evaluating a limit position at the mid point of each transition edge.\n"
"\n"
"    tessOuterLo = vec4(0);\n"
"    tessOuterHi = vec4(0);\n"
"\n"
"    vec3 corners[4];\n"
"    vec3 midpoints[4];\n"
"\n"
"    int transitionMask = OsdGetPatchTransitionMask(patchParam);\n"
"\n"
"#if defined OSD_PATCH_ENABLE_SINGLE_CREASE\n"
"    corners[0] = OsdEvalBezier(cpBezier, patchParam, vec2(0.0, 0.0));\n"
"    corners[1] = OsdEvalBezier(cpBezier, patchParam, vec2(1.0, 0.0));\n"
"    corners[2] = OsdEvalBezier(cpBezier, patchParam, vec2(1.0, 1.0));\n"
"    corners[3] = OsdEvalBezier(cpBezier, patchParam, vec2(0.0, 1.0));\n"
"\n"
"    midpoints[0] = ((transitionMask & 8) == 0) ? vec3(0) :  \n"
"        OsdEvalBezier(cpBezier, patchParam, vec2(0.0, 0.5));\n"
"    midpoints[1] = ((transitionMask & 1) == 0) ? vec3(0) :  \n"
"        OsdEvalBezier(cpBezier, patchParam, vec2(0.5, 0.0));\n"
"    midpoints[2] = ((transitionMask & 2) == 0) ? vec3(0) :  \n"
"        OsdEvalBezier(cpBezier, patchParam, vec2(1.0, 0.5));\n"
"    midpoints[3] = ((transitionMask & 4) == 0) ? vec3(0) :\n"
"        OsdEvalBezier(cpBezier, patchParam, vec2(0.5, 1.0));\n"
"#else\n"
"    corners[0] = cpBezier[ 0].P;\n"
"    corners[1] = cpBezier[ 3].P;\n"
"    corners[2] = cpBezier[15].P;\n"
"    corners[3] = cpBezier[12].P;\n"
"\n"
"    midpoints[0] = ((transitionMask & 8) == 0) ? vec3(0) :\n"
"        Osd_EvalBezierCurveMidPoint(\n"
"            cpBezier[0].P, cpBezier[4].P, cpBezier[8].P, cpBezier[12].P);\n"
"    midpoints[1] = ((transitionMask & 1) == 0) ? vec3(0) :\n"
"        Osd_EvalBezierCurveMidPoint(\n"
"            cpBezier[0].P, cpBezier[1].P, cpBezier[2].P, cpBezier[3].P);\n"
"    midpoints[2] = ((transitionMask & 2) == 0) ? vec3(0) :\n"
"        Osd_EvalBezierCurveMidPoint(\n"
"            cpBezier[3].P, cpBezier[7].P, cpBezier[11].P, cpBezier[15].P);\n"
"    midpoints[3] = ((transitionMask & 4) == 0) ? vec3(0) :\n"
"        Osd_EvalBezierCurveMidPoint(\n"
"            cpBezier[12].P, cpBezier[13].P, cpBezier[14].P, cpBezier[15].P);\n"
"#endif\n"
"\n"
"    Osd_GetTessLevelsFromPatchBoundaries4(corners, midpoints,\n"
"                                          patchParam, tessOuterLo, tessOuterHi);\n"
"}\n"
"\n"
"void\n"
"OsdEvalPatchBezierTriangleTessLevels(vec3 cv[15],\n"
"                 ivec3 patchParam, out vec4 tessOuterLo, out vec4 tessOuterHi)\n"
"{\n"
"    // Each edge of a transition patch is adjacent to one or two patches\n"
"    // at the next refined level of subdivision. When the patch control\n"
"    // points have been converted to the Bezier basis, the control points\n"
"    // at the corners are on the limit surface (since a Bezier patch\n"
"    // interpolates its corner control points). We can compute an adaptive\n"
"    // tessellation level for transition edges on the limit surface by\n"
"    // evaluating a limit position at the mid point of each transition edge.\n"
"\n"
"    tessOuterLo = vec4(0);\n"
"    tessOuterHi = vec4(0);\n"
"\n"
"    int transitionMask = OsdGetPatchTransitionMask(patchParam);\n"
"\n"
"    vec3 corners[3];\n"
"    corners[0] = cv[0];\n"
"    corners[1] = cv[4];\n"
"    corners[2] = cv[14];\n"
"\n"
"    vec3 midpoints[3];\n"
"    midpoints[0] = ((transitionMask & 4) == 0) ? vec3(0) :\n"
"        Osd_EvalQuarticBezierCurveMidPoint(cv[0], cv[5], cv[9], cv[12], cv[14]);\n"
"    midpoints[1] = ((transitionMask & 1) == 0) ? vec3(0) :\n"
"        Osd_EvalQuarticBezierCurveMidPoint(cv[0], cv[1], cv[2], cv[3], cv[4]);\n"
"    midpoints[2] = ((transitionMask & 2) == 0) ? vec3(0) :\n"
"        Osd_EvalQuarticBezierCurveMidPoint(cv[4], cv[8], cv[11], cv[13], cv[14]);\n"
"\n"
"    Osd_GetTessLevelsFromPatchBoundaries3(corners, midpoints,\n"
"                                          patchParam, tessOuterLo, tessOuterHi);\n"
"}\n"
"\n"
"// Round up to the nearest even integer\n"
"float OsdRoundUpEven(float x) {\n"
"    return 2*ceil(x/2);\n"
"}\n"
"\n"
"// Round up to the nearest odd integer\n"
"float OsdRoundUpOdd(float x) {\n"
"    return 2*ceil((x+1)/2)-1;\n"
"}\n"
"\n"
"// Compute outer and inner tessellation levels taking into account the\n"
"// current tessellation spacing mode.\n"
"void\n"
"OsdComputeTessLevels(inout vec4 tessOuterLo, inout vec4 tessOuterHi,\n"
"                     out vec4 tessLevelOuter, out vec2 tessLevelInner)\n"
"{\n"
"    // Outer levels are the sum of the Lo and Hi segments where the Hi\n"
"    // segments will have lengths of zero for non-transition edges.\n"
"\n"
"#if defined OSD_FRACTIONAL_EVEN_SPACING\n"
"    // Combine fractional outer transition edge levels before rounding.\n"
"    vec4 combinedOuter = tessOuterLo + tessOuterHi;\n"
"\n"
"    // Round the segments of transition edges separately. We will recover the\n"
"    // fractional parameterization of transition edges after tessellation.\n"
"\n"
"    tessLevelOuter = combinedOuter;\n"
"    if (tessOuterHi[0] > 0) {\n"
"        tessLevelOuter[0] =\n"
"            OsdRoundUpEven(tessOuterLo[0]) + OsdRoundUpEven(tessOuterHi[0]);\n"
"    }\n"
"    if (tessOuterHi[1] > 0) {\n"
"        tessLevelOuter[1] =\n"
"            OsdRoundUpEven(tessOuterLo[1]) + OsdRoundUpEven(tessOuterHi[1]);\n"
"    }\n"
"    if (tessOuterHi[2] > 0) {\n"
"        tessLevelOuter[2] =\n"
"            OsdRoundUpEven(tessOuterLo[2]) + OsdRoundUpEven(tessOuterHi[2]);\n"
"    }\n"
"    if (tessOuterHi[3] > 0) {\n"
"        tessLevelOuter[3] =\n"
"            OsdRoundUpEven(tessOuterLo[3]) + OsdRoundUpEven(tessOuterHi[3]);\n"
"    }\n"
"#elif defined OSD_FRACTIONAL_ODD_SPACING\n"
"    // Combine fractional outer transition edge levels before rounding.\n"
"    vec4 combinedOuter = tessOuterLo + tessOuterHi;\n"
"\n"
"    // Round the segments of transition edges separately. We will recover the\n"
"    // fractional parameterization of transition edges after tessellation.\n"
"    //\n"
"    // The sum of the two outer odd segment lengths will be an even number\n"
"    // which the tessellator will increase by +1 so that there will be a\n"
"    // total odd number of segments. We clamp the combinedOuter tess levels\n"
"    // (used to compute the inner tess levels) so that the outer transition\n"
"    // edges will be sampled without degenerate triangles.\n"
"\n"
"    tessLevelOuter = combinedOuter;\n"
"    if (tessOuterHi[0] > 0) {\n"
"        tessLevelOuter[0] =\n"
"            OsdRoundUpOdd(tessOuterLo[0]) + OsdRoundUpOdd(tessOuterHi[0]);\n"
"        combinedOuter = max(vec4(3), combinedOuter);\n"
"    }\n"
"    if (tessOuterHi[1] > 0) {\n"
"        tessLevelOuter[1] =\n"
"            OsdRoundUpOdd(tessOuterLo[1]) + OsdRoundUpOdd(tessOuterHi[1]);\n"
"        combinedOuter = max(vec4(3), combinedOuter);\n"
"    }\n"
"    if (tessOuterHi[2] > 0) {\n"
"        tessLevelOuter[2] =\n"
"            OsdRoundUpOdd(tessOuterLo[2]) + OsdRoundUpOdd(tessOuterHi[2]);\n"
"        combinedOuter = max(vec4(3), combinedOuter);\n"
"    }\n"
"    if (tessOuterHi[3] > 0) {\n"
"        tessLevelOuter[3] =\n"
"            OsdRoundUpOdd(tessOuterLo[3]) + OsdRoundUpOdd(tessOuterHi[3]);\n"
"        combinedOuter = max(vec4(3), combinedOuter);\n"
"    }\n"
"#else\n"
"    // Round equally spaced transition edge levels before combining.\n"
"    tessOuterLo = round(tessOuterLo);\n"
"    tessOuterHi = round(tessOuterHi);\n"
"\n"
"    vec4 combinedOuter = tessOuterLo + tessOuterHi;\n"
"    tessLevelOuter = combinedOuter;\n"
"#endif\n"
"\n"
"    // Inner levels are the averages the corresponding outer levels.\n"
"    tessLevelInner[0] = (combinedOuter[1] + combinedOuter[3]) * 0.5;\n"
"    tessLevelInner[1] = (combinedOuter[0] + combinedOuter[2]) * 0.5;\n"
"}\n"
"\n"
"void\n"
"OsdComputeTessLevelsTriangle(inout vec4 tessOuterLo, inout vec4 tessOuterHi,\n"
"                             out vec4 tessLevelOuter, out vec2 tessLevelInner)\n"
"{\n"
"    OsdComputeTessLevels(tessOuterLo, tessOuterHi,\n"
"                         tessLevelOuter, tessLevelInner);\n"
"\n"
"    // Inner level is the max of the three outer levels.\n"
"    tessLevelInner[0] = max(max(tessLevelOuter[0],\n"
"                                tessLevelOuter[1]),\n"
"                                tessLevelOuter[2]);\n"
"}\n"
"\n"
"void\n"
"OsdGetTessLevelsUniform(ivec3 patchParam,\n"
"                 out vec4 tessLevelOuter, out vec2 tessLevelInner,\n"
"                 out vec4 tessOuterLo, out vec4 tessOuterHi)\n"
"{\n"
"    OsdGetTessLevelsUniform(patchParam, tessOuterLo, tessOuterHi);\n"
"\n"
"    OsdComputeTessLevels(tessOuterLo, tessOuterHi,\n"
"                         tessLevelOuter, tessLevelInner);\n"
"}\n"
"\n"
"void\n"
"OsdGetTessLevelsUniformTriangle(ivec3 patchParam,\n"
"                 out vec4 tessLevelOuter, out vec2 tessLevelInner,\n"
"                 out vec4 tessOuterLo, out vec4 tessOuterHi)\n"
"{\n"
"    OsdGetTessLevelsUniformTriangle(patchParam, tessOuterLo, tessOuterHi);\n"
"\n"
"    OsdComputeTessLevelsTriangle(tessOuterLo, tessOuterHi,\n"
"                                 tessLevelOuter, tessLevelInner);\n"
"}\n"
"\n"
"void\n"
"OsdEvalPatchBezierTessLevels(OsdPerPatchVertexBezier cpBezier[16],\n"
"                 ivec3 patchParam,\n"
"                 out vec4 tessLevelOuter, out vec2 tessLevelInner,\n"
"                 out vec4 tessOuterLo, out vec4 tessOuterHi)\n"
"{\n"
"    OsdEvalPatchBezierTessLevels(cpBezier, patchParam,\n"
"                                 tessOuterLo, tessOuterHi);\n"
"\n"
"    OsdComputeTessLevels(tessOuterLo, tessOuterHi,\n"
"                         tessLevelOuter, tessLevelInner);\n"
"}\n"
"\n"
"void\n"
"OsdEvalPatchBezierTriangleTessLevels(vec3 cv[15],\n"
"                 ivec3 patchParam,\n"
"                 out vec4 tessLevelOuter, out vec2 tessLevelInner,\n"
"                 out vec4 tessOuterLo, out vec4 tessOuterHi)\n"
"{\n"
"    OsdEvalPatchBezierTriangleTessLevels(cv, patchParam,\n"
"                                         tessOuterLo, tessOuterHi);\n"
"\n"
"    OsdComputeTessLevelsTriangle(tessOuterLo, tessOuterHi,\n"
"                                 tessLevelOuter, tessLevelInner);\n"
"}\n"
"\n"
"void\n"
"OsdGetTessLevelsAdaptiveRefinedPoints(vec3 cpRefined[16], ivec3 patchParam,\n"
"                        out vec4 tessLevelOuter, out vec2 tessLevelInner,\n"
"                        out vec4 tessOuterLo, out vec4 tessOuterHi)\n"
"{\n"
"    OsdGetTessLevelsRefinedPoints(cpRefined, patchParam,\n"
"                                  tessOuterLo, tessOuterHi);\n"
"\n"
"    OsdComputeTessLevels(tessOuterLo, tessOuterHi,\n"
"                         tessLevelOuter, tessLevelInner);\n"
"}\n"
"\n"
"//  Deprecated -- prefer use of newer Bezier patch equivalent:\n"
"void\n"
"OsdGetTessLevelsLimitPoints(OsdPerPatchVertexBezier cpBezier[16],\n"
"                 ivec3 patchParam, out vec4 tessOuterLo, out vec4 tessOuterHi)\n"
"{\n"
"    OsdEvalPatchBezierTessLevels(cpBezier, patchParam, tessOuterLo, tessOuterHi);\n"
"}\n"
"\n"
"//  Deprecated -- prefer use of newer Bezier patch equivalent:\n"
"void\n"
"OsdGetTessLevelsAdaptiveLimitPoints(OsdPerPatchVertexBezier cpBezier[16],\n"
"                 ivec3 patchParam,\n"
"                 out vec4 tessLevelOuter, out vec2 tessLevelInner,\n"
"                 out vec4 tessOuterLo, out vec4 tessOuterHi)\n"
"{\n"
"    OsdGetTessLevelsLimitPoints(cpBezier, patchParam,\n"
"                                tessOuterLo, tessOuterHi);\n"
"\n"
"    OsdComputeTessLevels(tessOuterLo, tessOuterHi,\n"
"                         tessLevelOuter, tessLevelInner);\n"
"}\n"
"\n"
"//  Deprecated -- prefer use of newer Bezier patch equivalent:\n"
"void\n"
"OsdGetTessLevels(vec3 cp0, vec3 cp1, vec3 cp2, vec3 cp3,\n"
"                 ivec3 patchParam,\n"
"                 out vec4 tessLevelOuter, out vec2 tessLevelInner)\n"
"{\n"
"    vec4 tessOuterLo = vec4(0);\n"
"    vec4 tessOuterHi = vec4(0);\n"
"\n"
"#if defined OSD_ENABLE_SCREENSPACE_TESSELLATION\n"
"    tessOuterLo[0] = OsdComputeTessLevel(cp0, cp1);\n"
"    tessOuterLo[1] = OsdComputeTessLevel(cp0, cp3);\n"
"    tessOuterLo[2] = OsdComputeTessLevel(cp2, cp3);\n"
"    tessOuterLo[3] = OsdComputeTessLevel(cp1, cp2);\n"
"    tessOuterHi = vec4(0);\n"
"#else\n"
"    OsdGetTessLevelsUniform(patchParam, tessOuterLo, tessOuterHi);\n"
"#endif\n"
"\n"
"    OsdComputeTessLevels(tessOuterLo, tessOuterHi,\n"
"                         tessLevelOuter, tessLevelInner);\n"
"}\n"
"\n"
"#if defined OSD_FRACTIONAL_EVEN_SPACING || defined OSD_FRACTIONAL_ODD_SPACING\n"
"float\n"
"OsdGetTessFractionalSplit(float t, float level, float levelUp)\n"
"{\n"
"    // Fractional tessellation of an edge will produce n segments where n\n"
"    // is the tessellation level of the edge (level) rounded up to the\n"
"    // nearest even or odd integer (levelUp). There will be n-2 segments of\n"
"    // equal length (dx1) and two additional segments of equal length (dx0)\n"
"    // that are typically shorter than the other segments. The two additional\n"
"    // segments should be placed symmetrically on opposite sides of the\n"
"    // edge (offset).\n"
"\n"
"#if defined OSD_FRACTIONAL_EVEN_SPACING\n"
"    if (level <= 2) return t;\n"
"\n"
"    float base = pow(2.0,floor(log2(levelUp)));\n"
"    float offset = 1.0/(int(2*base-levelUp)/2 & int(base/2-1));\n"
"\n"
"#elif defined OSD_FRACTIONAL_ODD_SPACING\n"
"    if (level <= 1) return t;\n"
"\n"
"    float base = pow(2.0,floor(log2(levelUp)));\n"
"    float offset = 1.0/(((int(2*base-levelUp)/2+1) & int(base/2-1))+1);\n"
"#endif\n"
"\n"
"    float dx0 = (1.0 - (levelUp-level)/2) / levelUp;\n"
"    float dx1 = (1.0 - 2.0*dx0) / (levelUp - 2.0*ceil(dx0));\n"
"\n"
"    if (t < 0.5) {\n"
"        float x = levelUp/2 - round(t*levelUp);\n"
"        return 0.5 - (x*dx1 + int(x*offset > 1) * (dx0 - dx1));\n"
"    } else if (t > 0.5) {\n"
"        float x = round(t*levelUp) - levelUp/2;\n"
"        return 0.5 + (x*dx1 + int(x*offset > 1) * (dx0 - dx1));\n"
"    } else {\n"
"        return t;\n"
"    }\n"
"}\n"
"#endif\n"
"\n"
"float\n"
"OsdGetTessTransitionSplit(float t, float lo, float hi)\n"
"{\n"
"#if defined OSD_FRACTIONAL_EVEN_SPACING\n"
"    float loRoundUp = OsdRoundUpEven(lo);\n"
"    float hiRoundUp = OsdRoundUpEven(hi);\n"
"\n"
"    // Convert the parametric t into a segment index along the combined edge.\n"
"    float ti = round(t * (loRoundUp + hiRoundUp));\n"
"\n"
"    if (ti <= loRoundUp) {\n"
"        float t0 = ti / loRoundUp;\n"
"        return OsdGetTessFractionalSplit(t0, lo, loRoundUp) * 0.5;\n"
"    } else {\n"
"        float t1 = (ti - loRoundUp) / hiRoundUp;\n"
"        return OsdGetTessFractionalSplit(t1, hi, hiRoundUp) * 0.5 + 0.5;\n"
"    }\n"
"#elif defined OSD_FRACTIONAL_ODD_SPACING\n"
"    float loRoundUp = OsdRoundUpOdd(lo);\n"
"    float hiRoundUp = OsdRoundUpOdd(hi);\n"
"\n"
"    // Convert the parametric t into a segment index along the combined edge.\n"
"    // The +1 below is to account for the extra segment produced by the\n"
"    // tessellator since the sum of two odd tess levels will be rounded\n"
"    // up by one to the next odd integer tess level.\n"
"    float ti = round(t * (loRoundUp + hiRoundUp + 1));\n"
"\n"
"    if (ti <= loRoundUp) {\n"
"        float t0 = ti / loRoundUp;\n"
"        return OsdGetTessFractionalSplit(t0, lo, loRoundUp) * 0.5;\n"
"    } else if (ti > (loRoundUp+1)) {\n"
"        float t1 = (ti - (loRoundUp+1)) / hiRoundUp;\n"
"        return OsdGetTessFractionalSplit(t1, hi, hiRoundUp) * 0.5 + 0.5;\n"
"    } else {\n"
"        return 0.5;\n"
"    }\n"
"#else\n"
"    // Convert the parametric t into a segment index along the combined edge.\n"
"    float ti = round(t * (lo + hi));\n"
"\n"
"    if (ti <= lo) {\n"
"        return (ti / lo) * 0.5;\n"
"    } else {\n"
"        return ((ti - lo) / hi) * 0.5 + 0.5;\n"
"    }\n"
"#endif\n"
"}\n"
"\n"
"vec2\n"
"OsdGetTessParameterization(vec2 p, vec4 tessOuterLo, vec4 tessOuterHi)\n"
"{\n"
"    vec2 UV = p;\n"
"    if (p.x == 0 && tessOuterHi[0] > 0) {\n"
"        UV.y = OsdGetTessTransitionSplit(UV.y, tessOuterLo[0], tessOuterHi[0]);\n"
"    } else\n"
"    if (p.y == 0 && tessOuterHi[1] > 0) {\n"
"        UV.x = OsdGetTessTransitionSplit(UV.x, tessOuterLo[1], tessOuterHi[1]);\n"
"    } else\n"
"    if (p.x == 1 && tessOuterHi[2] > 0) {\n"
"        UV.y = OsdGetTessTransitionSplit(UV.y, tessOuterLo[2], tessOuterHi[2]);\n"
"    } else\n"
"    if (p.y == 1 && tessOuterHi[3] > 0) {\n"
"        UV.x = OsdGetTessTransitionSplit(UV.x, tessOuterLo[3], tessOuterHi[3]);\n"
"    }\n"
"    return UV;\n"
"}\n"
"\n"
"vec2\n"
"OsdGetTessParameterizationTriangle(vec3 p, vec4 tessOuterLo, vec4 tessOuterHi)\n"
"{\n"
"    vec2 UV = p.xy;\n"
"    if (p.x == 0 && tessOuterHi[0] > 0) {\n"
"        UV.y = OsdGetTessTransitionSplit(UV.y, tessOuterLo[0], tessOuterHi[0]);\n"
"    } else\n"
"    if (p.y == 0 && tessOuterHi[1] > 0) {\n"
"        UV.x = OsdGetTessTransitionSplit(UV.x, tessOuterLo[1], tessOuterHi[1]);\n"
"    } else\n"
"    if (p.z == 0 && tessOuterHi[2] > 0) {\n"
"        UV.x = OsdGetTessTransitionSplit(UV.x, tessOuterLo[2], tessOuterHi[2]);\n"
"        UV.y = 1.0 - UV.x;\n"
"    }\n"
"    return UV;\n"
"}\n"
"\n"