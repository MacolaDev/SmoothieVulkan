import sys
sys.path.append('D:\SoftwareDev_Projects\Smoothie\Smoothie')
a = "#" * 200
print(a)


import bpy
import struct
import io_helpers

def List_to_vector_3(input_list):
    x = input_list[0::3]
    y = input_list[1::3]
    z = input_list[2::3]
    return list(zip(x, y, z))

def List_to_vector_2(input_list):
    x = input_list[0::2]
    y = input_list[1::2]
    return list(zip(x, y))
    
def List_to_vector_4(input_list):
    r = input_list[0::4]
    g = input_list[1::4]
    b = input_list[2::4]
    a = input_list[3::4]
    return list(zip(r, g, b, a))

def cal_tangent_bitangent(mesh):
    loop_tangents_list = [0] * 3 * len(mesh.loops)
    loop_bitangent_list = [0] * 3 * len(mesh.loops)
        
    mesh.loops.foreach_get("tangent", loop_tangents_list) #Get tangent data from mesh
    loop_tangents_list = List_to_vector_3(loop_tangents_list) #Tuple data
    mesh.loops.foreach_get("bitangent", loop_bitangent_list) #Get bitangent data from mesh
    loop_bitangent_list = List_to_vector_3(loop_bitangent_list) #Tuple data


    vertex_indexes = [0] * len(mesh.loops)
    mesh.loops.foreach_get('vertex_index', vertex_indexes) #Get vertex indexes

    #Index every vector with vertex used by that loop
    indexed_tangent_list = list(zip(vertex_indexes, loop_tangents_list))
    indexed_bitangent_list = list(zip(vertex_indexes, loop_bitangent_list))

    #Sort data by index
    indexed_tangent_list = sorted(indexed_tangent_list, key = lambda element: element[0]) 
    indexed_bitangent_list = sorted(indexed_bitangent_list, key = lambda element: element[0]) 
    
    #Remove index   
    indexes_list = list(zip(*indexed_tangent_list))[0]
    loop_tangents_list = list(zip(*indexed_tangent_list))[1] 
    loop_bitangent_list = list(zip(*indexed_bitangent_list))[1]

    #Averages vectors per vertex. Check io_helpers.cpp for more info
    loop_tangents_list = io_helpers.vectorPerVertex(indexes_list, loop_tangents_list)
    loop_bitangent_list = io_helpers.vectorPerVertex(indexes_list, loop_bitangent_list)
    
    
    return (loop_tangents_list, loop_bitangent_list)

def packNormals(data):
    x_byte = int((1 + data[0]) * 127)&0xFF
    y_byte = int((1 + data[1]) * 127)&0xFF
    z_byte = int((1 + data[2]) * 127)&0xFF
    out = (((x_byte << 24) | (z_byte << 16)) | (y_byte << 8)) | 0
    return out

def get_data_from_collection(selected_collection):
    all_object_mesh_data = []
    collection_objects = selected_collection.objects
    num_of_meshes = 0
    for object in collection_objects:
               
        #if object on the scene is mesh
        if object.type == 'MESH':
            num_of_meshes+=1 
            mesh_data = object.data
        
            #Vertex coordinates data
            vertex_coordinates_list = [0] * len(mesh_data.vertices) * 3 #Create list with the size of all vertex data
            mesh_data.vertices.foreach_get('co', vertex_coordinates_list) #Get vertex coordinate data from object
            vertex_coordinates_list = List_to_vector_3(vertex_coordinates_list)
            
            #Index data
            loop_total_list = [0] * len(mesh_data.polygons)
            loop_start_list = [0] * len(mesh_data.polygons)
            mesh_data.polygons.foreach_get('loop_start', loop_start_list)
            mesh_data.polygons.foreach_get('loop_total', loop_total_list)
            
            if loop_total_list.count(3) != len(loop_start_list): #Checks if geometryhas only triangles
                raise Exception("Mesh has to have only have triangles. Triangulate mesh first.")
            else:
                triangle_indexes = [0] * len(mesh_data.loops)
                mesh_data.loops.foreach_get('vertex_index', triangle_indexes)
                triangle_indexes.reverse()
            
            #UV data
            if mesh_data.uv_layers: #if mesh has uv coordinates:
                first_layer = mesh_data.uv_layers[0].data  #Acces 1st mesh layer data
                
                first_uv_layer = [0] * len(mesh_data.loops) * 2
                first_layer.foreach_get("uv", first_uv_layer)
                first_uv_layer = List_to_vector_2(first_uv_layer)
                indexes = [0] * len(mesh_data.loops)
                mesh_data.loops.foreach_get('vertex_index', indexes) #Get vertex indexez
                indexed_uv_coordinates = list(zip(indexes, first_uv_layer)) #Assigg vertex index from that loop to UV coordinate
                indexed_uv_coordinates = list(set(indexed_uv_coordinates)) #Removes duplicates
                indexed_uv_coordinates = sorted(indexed_uv_coordinates, key = lambda element: element[0]) #Sort UV coordinates by vertex indexe
                first_uv_layer = list(zip(*indexed_uv_coordinates))[1] #Extracts UV tuples from indexed list. 
                
                
                if len(indexed_uv_coordinates) != len(mesh_data.vertices):
                    raise Exception('Some vertices in UV map have more UV coordinates. Check if UV is unwrapped correctly.')
            else:
                raise Exception("Mesh has to have UV layer")
            

            #Normals
            vertex_normals_list = [0] * 3 * len(mesh_data.vertices)
            mesh_data.vertices.foreach_get('normal', vertex_normals_list) #Get normals from mesh
            vertex_normals_list = List_to_vector_3(vertex_normals_list) #Tuple them

            mesh_data.calc_tangents(uvmap = mesh_data.uv_layers[0].name) #Calculate tangent and bitangent data
            data = cal_tangent_bitangent(mesh_data)
            vertex_tangents_list = data[0]
            vertex_bitangents_list = data[1]

            #Vertex color data
            if mesh_data.color_attributes:
                has_colour_layer = True
                blender_color_layer = mesh_data.color_attributes[0]
                if((blender_color_layer.data_type == "BYTE_COLOR") and (blender_color_layer.domain == "POINT")):
                    color_layer = [0] * len(mesh_data.vertices) * 4
                    blender_color_layer.data.foreach_get("color", color_layer) #Get flattered data from first colour layer
                    color_layer = List_to_vector_4(color_layer) #unflatter it
                else:
                    raise Exception("Domain of a color attribute must be POINT and data type must be BYTE_COLOR")
                
                # Debbuging
                # print("vertex color type: ", mesh_data.color_attributes[0].data_type)
                # print("vertex domain: ", mesh_data.color_attributes[0].domain)
                # for i, c in enumerate(color_layer):
                #     print(i, c)
                
            else:
                has_colour_layer = False
                color_layer = None
                
            object_mesh_data = {
                'object_name': object.name,
                'vertex_coordinates':vertex_coordinates_list,
                'vertex_normals': vertex_normals_list,
                'vertex_tangents': vertex_tangents_list,
                'vertex_bitangents': vertex_bitangents_list,
                'vertex_uvs':first_uv_layer,
                'triangle_index':triangle_indexes,
                'has_colour_layer':has_colour_layer,
                'colour_layer':color_layer
                }
            all_object_mesh_data.append(object_mesh_data)
    return all_object_mesh_data


#vertex buffer type, number of vertices, vertex pointer, number of indices, indices pointer

selected_collection = bpy.context.collection
data = get_data_from_collection(selected_collection)[0]


buffer_type = "xyznuvtb"

number_of_vertices = struct.pack("I", len(data["vertex_coordinates"]))
number_of_indexes = struct.pack("I", len(data["triangle_index"]))

vertex_buffer_size = len(data["vertex_coordinates"]) * 32  #xyznuvtb
index_buffer_size = len(data["triangle_index"]) * 2 #short, 2 bytes long 

vertices_file_offset = struct.pack("I", 20) #offset in bytes from start of file
indices_file_offset = struct.pack("I", (20 + vertex_buffer_size))

file = open("D:\\SoftwareDev_Projects\\Smoothie\\Smoothie\\House.geometry", "wb")

if buffer_type == "xyznuvtb":
    file.write(struct.pack("I", 0))

file.write(number_of_vertices)
file.write(vertices_file_offset)
file.write(number_of_indexes)
file.write(indices_file_offset)

for i in range(len(data["vertex_coordinates"])):
    x = data["vertex_coordinates"][i][0]
    y = data["vertex_coordinates"][i][1]
    z = data["vertex_coordinates"][i][2]

    u = data["vertex_uvs"][i][0]
    v = data["vertex_uvs"][i][1]

    packed_normals = packNormals(data["vertex_normals"][i])
    packed_tangent = packNormals(data["vertex_tangents"][i])
    packed_bitangent = packNormals(data["vertex_bitangents"][i])

    if i == 3347:
        print(i, "(", data["vertex_normals"][i][0], data["vertex_normals"][i][2], data["vertex_normals"][i][1], ")\n",
                 "(", data["vertex_tangents"][i][0], data["vertex_tangents"][i][2], data["vertex_tangents"][i][1], ")\n",
                 "(", data["vertex_bitangents"][i][0], data["vertex_bitangents"][i][2], data["vertex_bitangents"][i][1]), ")"
    file.write(struct.pack("3f", x, z, y))
    file.write(struct.pack(">I", packed_normals))
    file.write(struct.pack("2f", u, v))
    file.write(struct.pack(">2I", packed_tangent, packed_bitangent))
    
    
for i in data["triangle_index"]:
    file.write(struct.pack("I", i))


file.close()