#include <spirv_glsl.hpp>
#include <argh.h>
#include <string>
#include <vector>
#include <cstdint>
#include "filesystem_utils.h"
#include "string_utils.h"
#include "shaderdata.h"
#include <cstdio>
#include <codecvt>

std::string baseTypeToString(spirv_cross::SPIRType::BaseType type);

void createUniformBlockForResource(InputBlock* outBlock, spirv_cross::Resource res, spirv_cross::CompilerGLSL& compiler)
{
	uint32_t id = res.id;
	std::vector<spirv_cross::BufferRange> ranges = compiler.get_active_buffer_ranges(id);
	const spirv_cross::SPIRType& ub_type = compiler.get_type(res.base_type_id);

	outBlock->name = res.name;
	for (auto& range : ranges)
	{
		BlockMember mem;
		mem.name = compiler.get_member_name(res.base_type_id, range.index);
		mem.size = range.range;
		mem.offset = range.offset;

		auto type = compiler.get_type(res.type_id);

		auto baseType = type.basetype;
		std::string baseTypeString = baseTypeToString(baseType);
		std::string mName = compiler.get_member_name(res.base_type_id, range.index);

		outBlock->members.push_back(mem);
	}

	outBlock->size = compiler.get_declared_struct_size(ub_type);
	outBlock->binding = compiler.get_decoration(res.id, spv::DecorationBinding);
	outBlock->set = compiler.get_decoration(res.id, spv::DecorationDescriptorSet);
	outBlock->arrayLen = 1;
	outBlock->type = BlockType::UNIFORM;
}

void createSeparateSamplerBlockForResource(InputBlock* outBlock, spirv_cross::Resource res, spirv_cross::CompilerGLSL& compiler)
{
	uint32_t id = res.id;

	outBlock->name = res.name;
	outBlock->set = compiler.get_decoration(res.id, spv::DecorationDescriptorSet);
	outBlock->binding = compiler.get_decoration(res.id, spv::DecorationBinding);

	//.array is an array of uint32_t array sizes (for cases of more than one array)
	outBlock->arrayLen = compiler.get_type(res.type_id).array[0];

	outBlock->type = outBlock->arrayLen == 1 ? BlockType::SAMPLER : BlockType::SAMPLERARRAY;
}

void createSeparateTextureBlockForResource(InputBlock* outBlock, spirv_cross::Resource res, spirv_cross::CompilerGLSL& compiler)
{
	uint32_t id = res.id;

	outBlock->name = res.name;
	outBlock->set = compiler.get_decoration(res.id, spv::DecorationDescriptorSet);
	outBlock->binding = compiler.get_decoration(res.id, spv::DecorationBinding);

	outBlock->arrayLen = 1;

	if (compiler.get_type(res.type_id).array.size() > 0)
	{
		outBlock->arrayLen = compiler.get_type(res.type_id).array[0];
	}
	outBlock->type = outBlock->arrayLen == 1 ? BlockType::SEPARATETEXTURE : BlockType::TEXTUREARRAY;
}

void createTextureBlockForResource(InputBlock* outBlock, spirv_cross::Resource res, spirv_cross::CompilerGLSL& compiler)
{
	uint32_t id = res.id;

	outBlock->name = res.name;
	outBlock->set = compiler.get_decoration(res.id, spv::DecorationDescriptorSet);
	outBlock->binding = compiler.get_decoration(res.id, spv::DecorationBinding);
	outBlock->arrayLen = 1;
	outBlock->type = BlockType::TEXTURE;
}

int main(int argc, const char** argv) {

  argh::parser cmdl(argv);
  if (!cmdl(2)) {
    printf("ShaderPipeline: usage: ShaderPipeline <SPIR-V input dir> <reflection output dir> \n");
  }

  std::string shaderInPath = cmdl[1];
  std::string reflOutPath = cmdl[2];

  printf("Compiling reflections: \n\tSPV dir is %s, \n\tREFL dir is %s\n", shaderInPath.c_str(), reflOutPath.c_str());

  makeDirectoryRecursive(makeFullPath(reflOutPath));

  std::vector<std::string> inputFiles = getFilesInDirectory(shaderInPath);

	// generate reflection files for all built shaders
	{
		for (uint32_t i = 0; i < inputFiles.size(); ++i)
		{
			// Dont read anything that's not SPIR-V
      if (inputFiles[i].find(".spv") == std::string::npos) continue;

			ShaderData data = {};

      std::string spvPath = makeFullPath(inputFiles[i]);
      std::string reflPath = spvPath;
      findReplace(reflPath, std::string(".spv"), std::string(".refl"));

      printf("\tReflecting spv: %s\n", spvPath.c_str());
      fflush(stdout);
      FILE* shaderFile = fopen(spvPath.c_str(), "rb");
			assert(shaderFile);

			fseek(shaderFile, 0, SEEK_END);
			size_t filesize = ftell(shaderFile);
			size_t wordSize = sizeof(uint32_t);
			size_t wordCount = filesize / wordSize;
			rewind(shaderFile);

			uint32_t* ir = (uint32_t*)malloc(sizeof(uint32_t) * wordCount);

			fread(ir, filesize, 1, shaderFile);
			fclose(shaderFile);

			spirv_cross::CompilerGLSL glsl(ir, wordCount);

			spirv_cross::ShaderResources resources = glsl.get_shader_resources();

			for (spirv_cross::Resource res : resources.push_constant_buffers)
			{
				createUniformBlockForResource(&data.pushConstants, res, glsl);
			}

			data.descriptorSets.resize(resources.uniform_buffers.size() + resources.sampled_images.size() + resources.separate_images.size() + resources.separate_samplers.size());

			uint32_t idx = 0;
			for (spirv_cross::Resource res : resources.uniform_buffers)
			{
				createUniformBlockForResource(&data.descriptorSets[idx++], res, glsl);
			}

			for (spirv_cross::Resource res : resources.sampled_images)
			{
				createTextureBlockForResource(&data.descriptorSets[idx++], res, glsl);
			}

			for (spirv_cross::Resource res : resources.separate_images)
			{
				createSeparateTextureBlockForResource(&data.descriptorSets[idx++], res, glsl);
			}

			for (spirv_cross::Resource res : resources.separate_samplers)
			{
				createSeparateSamplerBlockForResource(&data.descriptorSets[idx++], res, glsl);
			}

			std::sort(data.descriptorSets.begin(), data.descriptorSets.end(), [](const InputBlock& lhs, const InputBlock& rhs)
			{
				if (lhs.set != rhs.set) return lhs.set < rhs.set;
				return lhs.binding < rhs.binding;
			});


			//write out material
			std::string shader = getReflectionString(data);
      FILE* file = fopen(reflPath.c_str(), "w");
			int results = fputs(shader.c_str(), file);
			assert(results != EOF);
			fclose(file);
		}
	}

	printf("Reflections generated successfully.\n");
	return 0;
}

std::string baseTypeToString(spirv_cross::SPIRType::BaseType type)
{
	std::string names[] = {
		"Unknown",
		"Void",
		"Boolean",
		"Char",
		"Int",
		"UInt",
		"Int64",
		"UInt64",
		"AtomicCounter",
		"Float",
		"Double",
		"Struct",
		"Image",
		"SampledImage",
		"Sampler"
	};

	return names[type];

}
