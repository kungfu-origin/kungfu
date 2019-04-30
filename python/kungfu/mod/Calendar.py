import pycalendar
import os

class Calendar:
    def __init__(self, base_dir):
        self.base_dir = base_dir
        self.name = "calendar_service"
        self.service = None
        calendar_path = self.base_dir + "/calendar"
        if not os.path.exists(calendar_path):
            os.makedirs(calendar_path)

    def start(self):
        self.service = pycalendar.CalendarService(self.base_dir)
        self.service.run()
        self.service.join()

    def stop(self):
        self.service.stop()