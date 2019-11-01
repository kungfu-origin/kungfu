
from .utils import object_as_dict

class Registry:
    _id_registry = {}
    _name_registry = {}
    _cls_registry = {}

    @classmethod
    def register(cls, type_id, type_name, type_class, writer_hook, reader_hook):
        if type_id in cls._cls_registry:
            raise Exception("type id {} already registered".format(type_id))
        if type_name in cls._name_registry:
            raise Exception("type name {} already registered".format(type_name))
        if type_class in cls._cls_registry:
            raise Exception("type cls {} already registered".format(type_class))
        meta = {"type_id":type_id, "type_name": type_name, "type_cls": type_class, "writer_hook": writer_hook, "reader_hook": reader_hook}
        cls._id_registry[type_id] = meta
        cls._name_registry[type_name] = meta
        cls._cls_registry[type_class] = meta

    @classmethod
    def type_ids(cls):
        return list(cls._id_registry.keys())

    @classmethod
    def type_names(cls):
        return [d["type_name"] for d in cls._id_registry.values()]

    @classmethod
    def type_classes(cls):
        return [d["type_class"] for d in cls._id_registry.values()]

    @classmethod
    def get_reader_hook(cls, type_id):
        if type_id in cls._id_registry:
            return cls._id_registry[type_id]["reader_hook"]
        return None

    @classmethod
    def get_writer_hook(cls, type_class):
        if type_class in cls._cls_registry:
            return cls._cls_registry[type_class]["writer_hook"]
        return None

    @classmethod
    def types(cls):
        return list(cls._id_registry.values())

    @classmethod
    def meta_from_name(cls, type_name):
        return cls._name_registry[type_name]

def monkey_patch():
    import pyyjj
    def get_data(event):
        hook = Registry.get_reader_hook(event.msg_type)
        if hook is None:
            raise Exception("failed to find reader hook for type id {}".format(event.msg_type))
        return hook(event)

    def write_data(writer, trigger_time, data):
        hook = Registry.get_writer_hook(type(data))
        if hook is None:
            raise Exception("failed to find writer hook for type class {}".format(type(data)))
        return hook(writer, trigger_time, data)

    def frame_as_dict(frame):
        data = get_data(frame)
        return {"source": frame.source,
                "dest": frame.dest,
                "trigger_time": frame.trigger_time,
                "gen_time": frame.gen_time,
                "msg_type": frame.msg_type,
                "data": object_as_dict(data)}
    pyyjj.frame.data = property(get_data)
    pyyjj.frame.as_dict = frame_as_dict
    pyyjj.writer.write_data = write_data

