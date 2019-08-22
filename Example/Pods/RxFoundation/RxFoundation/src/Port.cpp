//
//  Port.cpp
//  RxFoundation
//
//  Created by closure on 5/9/17.
//  Copyright © 2017 closure. All rights reserved.
//

#include <RxFoundation/Port.hpp>

using namespace Rx;

#if DEPLOYMENT_TARGET_MACOSX

RxPrivate uint32_t Port::processPortCount(void) {
    ipc_info_space_t info;
    ipc_info_name_array_t table = 0;
    mach_msg_type_number_t tableCount = 0;
    ipc_info_tree_name_array_t tree = 0;
    mach_msg_type_number_t treeCount = 0;
    
    kern_return_t ret = mach_port_space_info(mach_task_self(), &info, &table, &tableCount, &tree, &treeCount);
    if (ret != KERN_SUCCESS) {
        return (uint32_t)0;
    }
    if (table != NULL) {
        ret = vm_deallocate(mach_task_self(), (vm_address_t)table, tableCount * sizeof(*table));
        (void)ret;
    }
    if (tree != NULL) {
        ret = vm_deallocate(mach_task_self(), (vm_address_t)tree, treeCount * sizeof(*tree));
        (void)ret;
    }
    return (uint32_t)tableCount;
}

Port Port::allocate() {
    value_type value = value_type();
    kern_return_t ret = mach_port_allocate(mach_task_self(), MACH_PORT_RIGHT_RECEIVE, &value);
    if (KERN_SUCCESS != ret) {
        value = MACH_PORT_NULL;
    }
    ret = mach_port_insert_right(mach_task_self(), value, value, MACH_MSG_TYPE_MAKE_SEND);
    if (KERN_SUCCESS != ret) {
        CRASH("*** Unable to set send right on mach port. (%d) ***", ret);
    }
    mach_port_limits_t limits;
    limits.mpl_qlimit = 1;
    ret = mach_port_set_attributes(mach_task_self(), value, MACH_PORT_LIMITS_INFO, (mach_port_info_t)&limits, MACH_PORT_LIMITS_INFO_COUNT);
    if (KERN_SUCCESS != ret) CRASH("*** Unable to set attributes on mach port. (%d) ***", ret);
    return Port(value);
}

Port::Port(Port::value_type value) : PortBase<Port::value_type>(value) {
}

Port::Port(const Port &value) : PortBase<mach_port_t>(value.getValue()) {
}

Port::Port(Port &&value) : PortBase<mach_port_t>(value.getValue()) {
}

Port::~Port() {
}

Port Port::Null(MACH_PORT_NULL);

uint32_t Port::send(uint32_t msg_id, OptionFlags options, uint32_t timeout) {
    kern_return_t result;
    mach_msg_header_t header;
    header.msgh_bits = MACH_MSGH_BITS(MACH_MSG_TYPE_COPY_SEND, 0);
    header.msgh_size = sizeof(mach_msg_header_t);
    header.msgh_remote_port = getValue();
    header.msgh_local_port = MACH_PORT_NULL;
    header.msgh_id = msg_id;
    result = mach_msg(&header, (mach_msg_option_t)(MACH_SEND_MSG | options), header.msgh_size, 0, MACH_PORT_NULL, timeout, MACH_PORT_NULL);
    if (result == MACH_SEND_TIMED_OUT) {
        mach_msg_destroy(&header);
    }
    return result;
}

PortSet PortSet::allocate() {
    value_type value;
    kern_return_t ret = mach_port_allocate(mach_task_self(), MACH_PORT_RIGHT_PORT_SET, &value);
    if (KERN_SUCCESS != ret) { __THE_SYSTEM_HAS_NO_PORT_SETS_AVAILABLE__(ret); }
    return PortSet((KERN_SUCCESS == ret) ? value : MACH_PORT_NULL);
}

kern_return_t PortSet::insert(const Port &port) {
    if (Port::Null == port) {
        return -1;
    }
    return mach_port_insert_member(mach_task_self(), port.getValue(), getValue());
}

kern_return_t PortSet::remove(const Port &port) {
    if (Port::Null == port) {
        return -1;
    }
    return mach_port_extract_member(mach_task_self(), port.getValue(), getValue());
}

void PortSet::swap(PortSet &value) {
    PortBase::swap(value);
}

PortSet::~PortSet() {
    const kern_return_t ret = mach_port_mod_refs(mach_task_self(), getValue(), MACH_PORT_RIGHT_PORT_SET, -1);
    if (ret != KERN_SUCCESS) {
        //                    CFLog(kCFLogLevelError, CFSTR("error (%d - %s) while trying to free port set: %d"), ret, mach_error_string(ret), portSet);
    }
}

PortSet::PortSet(value_type value) : Port(value) {
}

PortSet::PortSet(PortSet &&value) : Port(value.getValue()) {
}

#endif
