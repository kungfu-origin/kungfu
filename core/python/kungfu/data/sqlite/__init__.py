
import os
import sys
from contextlib import contextmanager
from sqlalchemy import Column, ForeignKey, Integer, String
from sqlalchemy.ext.declarative import declarative_base
from sqlalchemy.orm import relationship
from sqlalchemy import create_engine
import json
from sqlalchemy import TypeDecorator, types
from sqlalchemy.orm import sessionmaker

Base = declarative_base()

class Json(TypeDecorator):
    @property
    def python_type(self):
        return object

    impl = types.String

    def process_bind_param(self, value, dialect):
        return json.dumps(value)

    def process_literal_param(self, value, dialect):
        return value

    def process_result_value(self, value, dialect):
        try:
            return json.loads(value)
        except (ValueError, TypeError):
            return None

class Task(Base):
    __tablename__ = 'kungfu_task'
    name = Column(String(250), nullable=False, primary_key=True)
    task_type = Column(String(250))
    config = Column(Json, nullable=False)

# Create an engine that stores data in the local directory's
engine = create_engine('sqlite:///{}/global/task.db'.format(os.environ['KF_HOME']))

# Create all tables in the engine. This is equivalent to "Create Table"
# statements in raw SQL.
Base.metadata.create_all(engine)
Session = sessionmaker(bind=engine)

@contextmanager
def session_scope():
    """Provide a transactional scope around a series of operations."""
    session = Session()
    try:
        yield session
        session.commit()
    except:
        session.rollback()
        raise
    finally:
        session.close()

def get_task_config(task_name):
    with session_scope() as session:
        task = session.query(Task).get(task_name)
        if task:
            return task.config
        else:
            return {}

def set_task_config(task_name, config):
    with session_scope() as session:
        task = session.query(Task).get(task_name)
        if task is None:
            task = Task(name = task_name, config = config)
            session.add(task)
        else:
            task.config = config