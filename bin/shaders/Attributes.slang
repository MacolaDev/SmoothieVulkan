#pragma once

// Disables reflecting a struct or a parameter.
[__AttributeUsage(_AttributeTargets.Struct)]
struct NoReflectionAttribute{}

//Mandatory for all shaders to be set.
//With this we identify shader inside a file.
[__AttributeUsage(_AttributeTargets.Function)]
struct TargetPipelineAttribute
{
    string pipeline;
};