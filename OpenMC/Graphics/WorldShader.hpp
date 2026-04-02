#pragma once

#include <LE/LegendEngine.hpp>

namespace mc
{
	class WorldShader
	{
	public:
		WorldShader();

		le::Shader& Get();
	private:
		le::Scope<le::Shader> m_Shader;
	};
}