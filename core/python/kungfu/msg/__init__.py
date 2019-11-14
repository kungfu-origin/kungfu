
from .utils import object_as_dict
import sys
import json

class Registry:
    _registry = {}

    @classmethod
    def register(cls, type_id, type_name, type_class):
        if type_id in cls._registry:
            raise Exception("type id {} already registered".format(type_id))
        meta = {"id":type_id, "name": type_name, "cls": type_class}
        cls._registry[type_id] = meta

    @classmethod
    def type_ids(cls):
        return list(cls._registry.keys())

    @classmethod
    def type_names(cls):
        return list([d["name"] for d in cls._registry.values()])

    @classmethod
    def types(cls):
        return list(cls._registry.values())

    @classmethod
    def get_cls(cls, type_id):
        if type_id in cls._registry:
            return cls._registry[type_id]["cls"]
        return None

    @classmethod
    def meta_from_name(cls, typename):
        for m in cls._registry.values():
            if m["name"] == typename:
                return m
        return None

def monkey_patch():
    try:
        import pyyjj
    except ImportError as err:
        return
    def get_data(event):
        cls = Registry.get_cls(event.msg_type)
        if cls is None:
            return None
        if cls == str:
            return event.data_as_string
        else:
            addr = event.data_address
            return cls.from_raw_address(addr)

    def write_data(writer, trigger_time, msg_type, data):
        if isinstance(data, str):
            return writer.write_str(trigger_time, msg_type, data)
        else:
            return writer.write_raw(trigger_time, msg_type, data.raw_address, sys.getsizeof(data))

    def frame_as_dict(frame):
        data = get_data(frame)
        if isinstance(data, str):
            data = json.loads(data)
        elif data is not None:
	        data = object_as_dict(data)
        return {"source": frame.source,
                "dest": frame.dest,
                "trigger_time": frame.trigger_time,
                "gen_time": frame.gen_time,
                "msg_type": frame.msg_type,
                "data": data}

    pyyjj.frame.data = property(get_data)
    pyyjj.frame.as_dict = frame_as_dict
    pyyjj.writer.write_data = write_data

