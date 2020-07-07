#ifdef UVW_AS_LIB
#include "async.h"
#endif

#include "config.h"


namespace uvw {


UVW_INLINE void AsyncHandle::sendCallback(uv_async_t *handle) {
    AsyncHandle &async = *(static_cast<AsyncHandle *>(handle->data));
    async.publish(AsyncEvent{});
}


UVW_INLINE bool AsyncHandle::init() {
    return initialize(&uv_async_init, &sendCallback);
}


UVW_INLINE void AsyncHandle::send() {
    invoke(&uv_async_send, get());
}

    template class UVW_EXTERN Resource<AsyncHandle, uv_async_t, AsyncEvent, CloseEvent, ErrorEvent>;
    template struct UVW_EXTERN Emitter<AsyncHandle, AsyncEvent, CloseEvent, ErrorEvent>::Connection<AsyncEvent>;
    template struct UVW_EXTERN Emitter<AsyncHandle, AsyncEvent, CloseEvent, ErrorEvent>::Connection<CloseEvent>;
    template struct UVW_EXTERN Emitter<AsyncHandle, AsyncEvent, CloseEvent, ErrorEvent>::Connection<ErrorEvent>;
    template struct UVW_EXTERN Emitter<AsyncHandle, AsyncEvent, CloseEvent, ErrorEvent>::Handler<AsyncEvent>;
    template struct UVW_EXTERN Emitter<AsyncHandle, AsyncEvent, CloseEvent, ErrorEvent>::Handler<CloseEvent>;
    template struct UVW_EXTERN Emitter<AsyncHandle, AsyncEvent, CloseEvent, ErrorEvent>::Handler<ErrorEvent>;

}
