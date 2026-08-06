#pragma once
#include "../Common.h"

namespace Smoothie 
{

    class Texture_Base
    {
    protected:
        VkImage m_Image = nullptr;
        VkImageView m_ImageView = nullptr;
        VmaAllocation m_Allocation = nullptr;

    public:
        virtual int create() = 0;
        virtual int recreate() {return 0;};
        virtual void destroy() = 0;

        inline VkImageView getImageView() const {return m_ImageView;}
        inline VkImage getImage() const{return m_Image;}
        inline VmaAllocation getAllocation() const {return m_Allocation;}

        virtual ~Texture_Base() = default;
    };



	class Texture2D: public Texture_Base
	{
	    int create() override;
	public:
		
		inline int create(const std::string& filepath) {this->filepath = filepath; return create();};
		void destroy() override;

		Texture2D() = default;



	private:
		std::string filepath;

		VkImage m_Image = nullptr;
		VkImageView m_ImageView = nullptr;
		VmaAllocation m_Allocation = nullptr;
	};

	//Empty 2D 256x256 texture with general layout with value (0.69f, 0.69f, 0.69f, 1.0f). 
	//It's men to be used as placeholder texture for any sampler.
	class DefaultTexture2D 
	{
		VkImageView imageView = nullptr;
		VkImage image = nullptr;
		VmaAllocation allocation = nullptr;

	public:
		int create();
		void destroy();

		inline VkImageView getImageView() const { return imageView; }
		inline VkImage getImage() const { return image; }
        inline VmaAllocation getAllocation() const { return allocation; }
	    DefaultTexture2D() = default;
	};

}

