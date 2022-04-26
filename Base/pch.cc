#include "pch.hh"

/// EA ALLOCATOR
void *operator new(size_t size)
{
    void *PTR = _aligned_offset_malloc(size, 16, 0);
    return PTR;
}

void *operator new[](size_t size)
{
    void *PTR = _aligned_offset_malloc(size, 16, 0);
    return PTR;
}

void *operator new[](size_t size, const char * /*name*/, int /*flags*/, unsigned /*debugFlags*/, const char * /*file*/, int /*line*/)
{
    void *PTR = _aligned_offset_malloc(size, 16, 0);
    return PTR;
}

void *operator new[](size_t size, size_t alignment, size_t /*alignmentOffset*/, const char * /*name*/, int /*flags*/, unsigned /*debugFlags*/,
                     const char * /*file*/, int /*line*/)
{
    void *PTR = _aligned_offset_malloc(size, alignment, 0);
    return PTR;
}

void *operator new(size_t size, size_t alignment)
{
    void *PTR = _aligned_offset_malloc(size, alignment, 0);
    return PTR;
}

void *operator new(size_t size, size_t alignment, const std::nothrow_t &) EA_THROW_SPEC_NEW_NONE()
{
    void *PTR = _aligned_offset_malloc(size, alignment, 0);
    return PTR;
}

void *operator new[](size_t size, size_t alignment)
{
    void *PTR = _aligned_offset_malloc(size, alignment, 0);
    return PTR;
}

void *operator new[](size_t size, size_t alignment, const std::nothrow_t &) EA_THROW_SPEC_NEW_NONE()
{
    void *PTR = _aligned_offset_malloc(size, alignment, 0);
    return PTR;
}

void operator delete[](void *p) noexcept
{
    _aligned_free(p);
}

void operator delete(void *p, std::size_t sz) EA_THROW_SPEC_DELETE_NONE()
{
    _aligned_free(p);
}

void operator delete[](void *p, std::size_t sz) EA_THROW_SPEC_DELETE_NONE()
{
    _aligned_free(p);
}

void operator delete(void *p) EA_THROW_SPEC_DELETE_NONE()
{
    _aligned_free(p);
}