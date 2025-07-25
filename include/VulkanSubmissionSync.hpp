#pragma once





namespace VRenderer
{
	enum class VulkanSubmissionSync
	{
		SIGNAL,
		WAIT,
		WAIT_PREP_FOR_PRESENTATION,
		PREP_FOR_PRESENTATION
	};
}