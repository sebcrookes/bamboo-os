#include "framebuffer.h"

#include "../inc/faults.h"

void framebuffer_init(bamboo_fb_t* fb, struct limine_framebuffer_request framebuffer_request) {
    if (framebuffer_request.response == NULL
     || framebuffer_request.response->framebuffer_count < 1) {
        faults_hang();
    }

    struct limine_framebuffer* lfb = framebuffer_request.response->framebuffers[0];

    fb->buffer = (uint32_t*) lfb->address;
    fb->width = lfb->width;
    fb->height = lfb->height;
    fb->pitch = lfb->pitch;
}
