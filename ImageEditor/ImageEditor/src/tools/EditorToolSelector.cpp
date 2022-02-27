#include "EditorToolSelector.h"

EditorToolSelector::EditorToolSelector()
	: current_tool(TOOLS::PENCIL), tool_size(1)
{

}

EditorToolSelector::~EditorToolSelector()
{

}

ImVec4 EditorToolSelector::GetColor() const
{
	return ImVec4(
		this->color.x * 255.0f,
		this->color.y * 255.0f,
		this->color.z * 255.0f,
		this->color.w * 255.0f
	);
}

ImVec4& EditorToolSelector::GetColorReference()
{
	return this->color;
}

ImVec4 EditorToolSelector::GetColorNormalized() const
{
	return this->color;
}