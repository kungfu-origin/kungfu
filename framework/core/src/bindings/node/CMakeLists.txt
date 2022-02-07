project(kungfu-node-binding)

message(STATUS "Configuring for node bindings")

use_node_addon_api()
use_libnode()

aux_source_directory(binding SOURCE_FILES_KUNGFU_NODE)
aux_source_directory(drone SOURCE_FILES_DRONE)

if ("${NODE_RUNTIME}" STREQUAL "node")
  set(KF_NODE_BINDING_NAME "kungfu_node")
endif ()

if ("${NODE_RUNTIME}" STREQUAL "electron")
  set(KF_NODE_BINDING_NAME "kungfu_electron")
endif ()

if (KF_NODE_BINDING_NAME)
  build_node_binding(${KF_NODE_BINDING_NAME} "${SOURCE_FILES_KUNGFU_NODE}")
endif ()

if ("${NODE_RUNTIME}" STREQUAL "node")
  build_node_binding("drone" "${SOURCE_FILES_DRONE}")
endif ()

if ("${NODE_RUNTIME}" STREQUAL "node" AND UNIX AND NOT APPLE)
  build_node_binding("kungfu_kfc" "${SOURCE_FILES_KUNGFU_NODE}" ${LIBNODE})
endif ()

if (NOT KF_NODE_BINDING_NAME)
  message(WARNING "Unknown node runtime ${NODE_RUNTIME}")
endif ()
