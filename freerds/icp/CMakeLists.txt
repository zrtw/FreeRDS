# FreeRDP X11 Server Next Generation
# FreeRDS cmake build script
#
# Copyright 2013 Thinstuff Technologies GmbH
# Copyright 2013 Bernhard Miklautz <bmiklautz@thinstuff.at>
#
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

set(MODULE_NAME "freerds-icp")
set(MODULE_PREFIX "FREERDS_ICP")
set(PBRPC_PREFIX "pbrpc")

PROTOBUFC_GENERATE_C(ICP_PROTOC_SRC ICP_PROTOC_HDRS ${CMAKE_CURRENT_SOURCE_DIR}/../../protocols/protobuf/ICP.proto)
PROTOBUFC_GENERATE_C(PBRPC_PROTOC_SRC PBRPC_PROTOC_HDRS ${CMAKE_CURRENT_SOURCE_DIR}/../../protocols/protobuf/pbRPC.proto)

set(PBRPC_SRC
  ${PBRPC_PREFIX}/pbrpc.c
	${PBRPC_PREFIX}/pbrpc_utils.c
	${PBRPC_PREFIX}/pipe_transport.c
	${PBRPC_PROTOC_HDRS}
	${PBRPC_PROTOC_SRC}
)

include_directories(${PBRPC_PREFIX})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(${MODULE_PREFIX}_SRCS
	icp.c
	icp_client_stubs.c
	icp_server_stubs.c
	${ICP_PROTOC_SRC}
	${ICP_PROTOC_HDRS}
	${PBRPC_SRC}
)

add_library(${MODULE_NAME} ${${MODULE_PREFIX}_SRCS})

set_complex_link_libraries(VARIABLE ${MODULE_PREFIX}_LIBS
	MONOLITHIC ${MONOLITHIC_BUILD}
	MODULE winpr
	MODULES winpr-utils winpr-file winpr-pipe)

target_link_libraries(${MODULE_NAME} ${${MODULE_PREFIX}_LIBS} ${PROTOBUFC_LIBRARIES})

install(TARGETS ${MODULE_NAME} DESTINATION ${CMAKE_INSTALL_LIBDIR})
