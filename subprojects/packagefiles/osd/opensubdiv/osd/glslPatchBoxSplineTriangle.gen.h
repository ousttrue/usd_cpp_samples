"//\n"
"//   Copyright 2018 Pixar\n"
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
"//----------------------------------------------------------\n"
"// Patches.VertexBoxSplineTriangle\n"
"//----------------------------------------------------------\n"
"#ifdef OSD_PATCH_VERTEX_BOX_SPLINE_TRIANGLE_SHADER\n"
"\n"
"layout(location = 0) in vec4 position;\n"
"OSD_USER_VARYING_ATTRIBUTE_DECLARE\n"
"\n"
"out block {\n"
"    ControlVertex v;\n"
"    OSD_USER_VARYING_DECLARE\n"
"} outpt;\n"
"\n"
"void main()\n"
"{\n"
"    outpt.v.position = position;\n"
"    OSD_PATCH_CULL_COMPUTE_CLIPFLAGS(position);\n"
"    OSD_USER_VARYING_PER_VERTEX();\n"
"}\n"
"\n"
"#endif\n"
"\n"
"//----------------------------------------------------------\n"
"// Patches.TessControlBoxSplineTriangle\n"
"//----------------------------------------------------------\n"
"#ifdef OSD_PATCH_TESS_CONTROL_BOX_SPLINE_TRIANGLE_SHADER\n"
"\n"
"patch out vec4 tessOuterLo, tessOuterHi;\n"
"\n"
"in block {\n"
"    ControlVertex v;\n"
"    OSD_USER_VARYING_DECLARE\n"
"} inpt[];\n"
"\n"
"out block {\n"
"    OsdPerPatchVertexBezier v;\n"
"    OSD_USER_VARYING_DECLARE\n"
"} outpt[15];\n"
"\n"
"layout(vertices = 15) out;\n"
"\n"
"void main()\n"
"{\n"
"    vec3 cv[12];\n"
"    for (int i=0; i<12; ++i) {\n"
"        cv[i] = inpt[i].v.position.xyz;\n"
"    }\n"
"\n"
"    ivec3 patchParam = OsdGetPatchParam(OsdGetPatchIndex(gl_PrimitiveID));\n"
"    OsdComputePerPatchVertexBoxSplineTriangle(\n"
"        patchParam, gl_InvocationID, cv, outpt[gl_InvocationID].v);\n"
"\n"
"    OSD_USER_VARYING_PER_CONTROL_POINT(gl_InvocationID, gl_InvocationID);\n"
"\n"
"#if defined OSD_ENABLE_SCREENSPACE_TESSELLATION\n"
"    // Wait for all basis conversion to be finished\n"
"    barrier();\n"
"#endif\n"
"    if (gl_InvocationID == 0) {\n"
"        vec4 tessLevelOuter = vec4(0);\n"
"        vec2 tessLevelInner = vec2(0);\n"
"\n"
"        OSD_PATCH_CULL(12);\n"
"\n"
"#if defined OSD_ENABLE_SCREENSPACE_TESSELLATION\n"
"        // Gather bezier control points to compute limit surface tess levels\n"
"        vec3 bezcv[15];\n"
"        for (int i=0; i<15; ++i) {\n"
"            bezcv[i] = outpt[i].v.P;\n"
"        }\n"
"        OsdEvalPatchBezierTriangleTessLevels(\n"
"                         bezcv, patchParam,\n"
"                         tessLevelOuter, tessLevelInner,\n"
"                         tessOuterLo, tessOuterHi);\n"
"#else\n"
"        OsdGetTessLevelsUniformTriangle(patchParam,\n"
"                        tessLevelOuter, tessLevelInner,\n"
"                        tessOuterLo, tessOuterHi);\n"
"\n"
"#endif\n"
"\n"
"        gl_TessLevelOuter[0] = tessLevelOuter[0];\n"
"        gl_TessLevelOuter[1] = tessLevelOuter[1];\n"
"        gl_TessLevelOuter[2] = tessLevelOuter[2];\n"
"\n"
"        gl_TessLevelInner[0] = tessLevelInner[0];\n"
"    }\n"
"}\n"
"\n"
"#endif\n"
"\n"
"//----------------------------------------------------------\n"
"// Patches.TessEvalBoxSplineTriangle\n"
"//----------------------------------------------------------\n"
"#ifdef OSD_PATCH_TESS_EVAL_BOX_SPLINE_TRIANGLE_SHADER\n"
"\n"
"layout(triangles) in;\n"
"layout(OSD_SPACING) in;\n"
"\n"
"patch in vec4 tessOuterLo, tessOuterHi;\n"
"\n"
"in block {\n"
"    OsdPerPatchVertexBezier v;\n"
"    OSD_USER_VARYING_DECLARE\n"
"} inpt[];\n"
"\n"
"out block {\n"
"    OutputVertex v;\n"
"    OSD_USER_VARYING_DECLARE\n"
"} outpt;\n"
"\n"
"void main()\n"
"{\n"
"    vec3 P = vec3(0), dPu = vec3(0), dPv = vec3(0);\n"
"    vec3 N = vec3(0), dNu = vec3(0), dNv = vec3(0);\n"
"\n"
"    OsdPerPatchVertexBezier cv[15];\n"
"    for (int i = 0; i < 15; ++i) {\n"
"        cv[i] = inpt[i].v;\n"
"    }\n"
"\n"
"    vec2 UV = OsdGetTessParameterizationTriangle(gl_TessCoord.xyz,\n"
"                                                 tessOuterLo,\n"
"                                                 tessOuterHi);\n"
"\n"
"    ivec3 patchParam = inpt[0].v.patchParam;\n"
"    OsdEvalPatchBezierTriangle(patchParam, UV, cv, P, dPu, dPv, N, dNu, dNv);\n"
"\n"
"    // all code below here is client code\n"
"    outpt.v.position = OsdModelViewMatrix() * vec4(P, 1.0f);\n"
"    outpt.v.normal = (OsdModelViewMatrix() * vec4(N, 0.0f)).xyz;\n"
"    outpt.v.tangent = (OsdModelViewMatrix() * vec4(dPu, 0.0f)).xyz;\n"
"    outpt.v.bitangent = (OsdModelViewMatrix() * vec4(dPv, 0.0f)).xyz;\n"
"#ifdef OSD_COMPUTE_NORMAL_DERIVATIVES\n"
"    outpt.v.Nu = dNu;\n"
"    outpt.v.Nv = dNv;\n"
"#endif\n"
"\n"
"    outpt.v.tessCoord = UV;\n"
"    outpt.v.patchCoord = OsdInterpolatePatchCoordTriangle(UV, patchParam);\n"
"\n"
"    OSD_USER_VARYING_PER_EVAL_POINT_TRIANGLE(UV, 4, 5, 8);\n"
"\n"
"    OSD_DISPLACEMENT_CALLBACK;\n"
"\n"
"    gl_Position = OsdProjectionMatrix() * outpt.v.position;\n"
"}\n"
"\n"
"#endif\n"
"\n"