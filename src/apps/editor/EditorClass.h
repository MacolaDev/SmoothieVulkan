#pragma once
#define SMOOTHIE_INCLUDE_EDITOR
#include "../Smoothie.h"

//Interface between engine and main drawing class of the engine :D
class EditorClass: public  Smoothie::DeferredRendering::Drawing
{
protected:
    void compute_post_processing(VkCommandBuffer command_buffer) const final;
    void present_pass(VkCommandBuffer command_buffer) const final;

public:
    int create() final;
    int resize_callback() final;
    void destroy() final;

private:
    Smoothie::DeferredRendering::Texture2D_Attachment m_RenderTarget;

    void begin_target_pass(VkCommandBuffer command_buffer) const;
    void end_target_pass(VkCommandBuffer command_buffer) const;

    mutable Smoothie::Editor::Core m_Editor;

};
