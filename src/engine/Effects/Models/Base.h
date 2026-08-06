#pragma once
#include "Common.h"

namespace Smoothie
{
    namespace DeferredRendering
    {
        class Model_Base
        {
        public:
            virtual int create() = 0;
            virtual int update() {return 0;};
            virtual void destroy() = 0;

            virtual ~Model_Base() = default;
        };
    }
}