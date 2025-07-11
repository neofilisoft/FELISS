import bpy
import os
import configparser

CONFIG_PATH = os.path.abspath("../../configuration.ini")

def read_engine_config():
    config = configparser.ConfigParser()
    config.read(CONFIG_PATH)
    aa = config.get("Graphics", "AntiAliasing", fallback="TAA")
    print(f"[FelissBlenderAPI] Current AA Mode: {aa}")
    return aa

def export_to_feliss_fbx(output_dir):
    if not os.path.exists(output_dir):
        os.makedirs(output_dir)

    for obj in bpy.context.selected_objects:
        name = obj.name.replace(" ", "_")
        filepath = os.path.join(output_dir, f"{name}.fbx")
        bpy.ops.export_scene.fbx(
            filepath=filepath,
            use_selection=True,
            apply_unit_scale=True,
            bake_space_transform=True
        )
        print(f"[FelissBlenderAPI] Exported {name} to {filepath}")

class ExportToFelissOperator(bpy.types.Operator):
    bl_idname = "export_scene.feliss_fbx"
    bl_label = "Export to Feliss Engine (.fbx)"

    def execute(self, context):
        export_to_feliss_fbx("C:/Feliss/Exports")  # Change path for cross-platform
        return {'FINISHED'}

class FelissPanel(bpy.types.Panel):
    bl_label = "Feliss Engine Tools"
    bl_idname = "VIEW3D_PT_feliss_panel"
    bl_space_type = 'VIEW_3D'
    bl_region_type = 'UI'
    bl_category = 'Feliss'

    def draw(self, context):
        layout = self.layout
        layout.operator("export_scene.feliss_fbx")
        layout.label(text=f"AA Mode: {read_engine_config()}")

classes = [ExportToFelissOperator, FelissPanel]

def register():
    for cls in classes:
        bpy.utils.register_class(cls)

def unregister():
    for cls in classes:
        bpy.utils.unregister_class(cls)
