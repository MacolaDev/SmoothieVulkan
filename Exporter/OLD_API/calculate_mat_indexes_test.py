#This file is used as a unit test for calc_mat_indexes function in SmoothieExporter. 
#It should be run as single script inside Blender.

import bpy
import SmoothieExporter

polygons = bpy.context.object.data.polygons

class mat_indices_data:
    vertex_index: list
    material_index: list
    
    def __init__(self, vertex_list, material_list):
        self.vertex_index = vertex_list
        self.material_index = material_list
        return None
    

vertex_data = [0] * 3 * len(polygons) #Assumed that mesh only has triangles.
material_data = [0] * len(polygons)

polygons.foreach_get("vertices", vertex_data)
polygons.foreach_get("material_index", material_data)

to_process = mat_indices_data(vertex_data, material_data)

exit_data = SmoothieExporter.calculate_mat_indexes(to_process)

print(len(exit_data))