#ifdef UVW_AS_LIB
#include "idle.h"
#endif

#include "config.h"


namespace uvw {


UVW_INLINE void IdleHandle::startCallback(uv_idle_t *handle) {
    IdleHandle &idle = *(static_cast<IdleHandle *>(handle->data));
    idle.publish(IdleEvent{});
}


UVW_INLINE bool IdleHandle::init() {
    return initialize(&uv_idle_init);
}


UVW_INLINE void IdleHandle::start() {
    invoke(&uv_idle_start, get(), &startCallback);
}


UVW_INLINE void IdleHandle::stop() {
    invoke(&uv_idle_stop, get());
}


    template class UVW_EXTERN Resource<IdleHandle, uv_idle_t , IdleEvent, CloseEvent, ErrorEvent>;
    template struct UVW_EXTERN Emitter<IdleHandle, IdleEvent, CloseEvent, ErrorEvent>::Connection<IdleEvent>;
    template struct UVW_EXTERN Emitter<IdleHandle, IdleEvent, CloseEvent, ErrorEvent>::Connection<CloseEvent>;
    template struct UVW_EXTERN Emitter<IdleHandle, IdleEvent, CloseEvent, ErrorEvent>::Connection<ErrorEvent>;
    template struct UVW_EXTERN Emitter<IdleHandle, IdleEvent, CloseEvent, ErrorEvent>::Handler<IdleEvent>;
    template struct UVW_EXTERN Emitter<IdleHandle, IdleEvent, CloseEvent, ErrorEvent>::Handler<CloseEvent>;
    template struct UVW_EXTERN Emitter<IdleHandle, IdleEvent, CloseEvent, ErrorEvent>::Handler<ErrorEvent>;


}
