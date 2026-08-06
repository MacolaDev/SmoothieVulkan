#Only used for performance tests :D

from pathlib import Path
import shutil

scripts_dir = Path(__file__).resolve().parent
root_dir = scripts_dir.parent
assets_dir = root_dir.joinpath("assets")
bin_dir = root_dir.joinpath("bin")


if assets_dir.exists() == False:
    raise Exception("Assets folder is not present: {}".format(assets_dir))

testing_dir = assets_dir.joinpath("brute_force_process_assets")
result_dir = bin_dir.joinpath("brute_force_process_assets")
testing_dir.mkdir(parents=True, exist_ok=True)

test_file_txt = """
static float4x4 projection = float4x4(0.0f);
static float4x4 view = float4x4(0.0f);
[shader("vertex")]
float4 main_vert(in float3 aPos)
{
    float4 frag_pos = mul(projection, mul(view, float4(aPos, 1.0f)));
    return frag_pos;
}
[shader("fragment")]
float4 main_frag()
{
    return float4(float3(0.0f), 1.0f);
}
"""

for i in range(1000):
    path = testing_dir.joinpath("test_file_{}.slang".format(i))
    f = open(path, "w")
    f.write(test_file_txt)
    f.close()


import process_assets
process_assets.process_assets()

shutil.rmtree(str(testing_dir))
shutil.rmtree(str(result_dir))