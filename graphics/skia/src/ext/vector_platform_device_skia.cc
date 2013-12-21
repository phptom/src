// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ext/vector_platform_device_skia.h"

#include "ext/bitmap_platform_device.h"
#include "include/core/SkClipStack.h"
#include "include/core/SkDraw.h"
#include "include/core/SkRect.h"
#include "include/core/SkRegion.h"
#include "include/core/SkScalar.h"

namespace skia {

static inline SkBitmap makeABitmap(int width, int height) {
  SkBitmap bitmap;
  bitmap.setConfig(SkBitmap::kNo_Config, width, height);
  return bitmap;
}

VectorPlatformDeviceSkia::VectorPlatformDeviceSkia(
    const SkISize& pageSize,
    const SkISize& contentSize,
    const SkMatrix& initialTransform)
    : SkPDFDevice(pageSize, contentSize, initialTransform) {
  SetPlatformDevice(this, this);
}

VectorPlatformDeviceSkia::~VectorPlatformDeviceSkia() {
}

bool VectorPlatformDeviceSkia::IsNativeFontRenderingAllowed() {
  return false;
}

PlatformSurface VectorPlatformDeviceSkia::BeginPlatformPaint() {
  // Even when drawing a vector representation of the page, we have to
  // provide a raster surface for plugins to render into - they don't have
  // a vector interface.  Therefore we create a BitmapPlatformDevice here
  // and return the context from it, then layer on the raster data as an
  // image in EndPlatformPaint.
  DCHECK(raster_surface_ == NULL);
#if defined(OS_WIN)
  raster_surface_ = BitmapPlatformDevice::create(width(),
                                                 height(),
                                                 false, /* not opaque */
                                                 NULL);
#elif defined(OS_POSIX) && !defined(OS_MACOSX)
  raster_surface_ = BitmapPlatformDevice::Create(width(),
                                                 height(),
                                                 false /* not opaque */);
#endif
  raster_surface_->unref();  // SkRefPtr and create both took a reference.

  SkCanvas canvas(raster_surface_.get());
  return raster_surface_->BeginPlatformPaint();
}

void VectorPlatformDeviceSkia::EndPlatformPaint() {
  DCHECK(raster_surface_ != NULL);
  SkPaint paint;
  // SkPDFDevice checks the passed SkDraw for an empty clip (only).  Fake
  // it out by setting a non-empty clip.
  SkDraw draw;
  SkRegion clip(SkIRect::MakeWH(width(), height()));
  draw.fClip=&clip;
  drawSprite(draw, raster_surface_->accessBitmap(false), 0, 0, paint);
  // BitmapPlatformDevice matches begin and end calls.
  raster_surface_->EndPlatformPaint();
  raster_surface_ = NULL;
}

#if defined(OS_WIN)
void VectorPlatformDeviceSkia::DrawToNativeContext(HDC dc,
                                                   int x,
                                                   int y,
                                                   const RECT* src_rect) {
  SkASSERT(false);
}
#elif defined(OS_MACOSX)
void VectorPlatformDeviceSkia::DrawToNativeContext(CGContext* context, int x,
    int y, const CGRect* src_rect) {
  SkASSERT(false);
}

CGContextRef VectorPlatformDeviceSkia::GetBitmapContext() {
  SkASSERT(false);
  return NULL;
}
#elif defined(OS_LINUX) || defined(OS_ANDROID) || defined(OS_OPENBSD)
void VectorPlatformDeviceSkia::DrawToNativeContext(
    PlatformSurface surface, int x, int y, const PlatformRect* src_rect) {
  // Should never be called on Linux.
  SkASSERT(false);
}
#endif

}  // namespace skia
