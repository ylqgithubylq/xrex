#include <XREXAll.hpp>

/*
 *	Use TutorialFramework to do the work.
 *	This is an implementation of the framework.
 */
class TutorialFrameworkXREXImplementation
{
	struct RenderingProcessContainer
		: XREX::RenderingProcess
	{
		TutorialFrameworkXREXImplementation& framework;

		RenderingProcessContainer(TutorialFrameworkXREXImplementation& framework)
			: framework(framework)
		{
		}
		virtual void RenderScene(XREX::SceneSP const& scene) override
		{
			if (framework.render != nullptr)
			{
				framework.render();
			}
		}
	};

public:
	TutorialFrameworkXREXImplementation(XREX::Size<XREX::uint32, 2> windowSize,
		std::function<void()> initialize,
		std::function<bool(double current, double delta)> update,
		std::function<void()> render)
		: initialize(std::move(initialize))
		, render(std::move(render))
	{
		XREX::Settings settings("../../"); // root directory
		settings.windowTitle = L"GL Tutorial";

		settings.renderingSettings.sampleCount = 1;

		settings.renderingSettings.left = 100;
		settings.renderingSettings.top = 100;
		settings.renderingSettings.width = windowSize.X();
		settings.renderingSettings.height = windowSize.Y();

		XREX::XREXContext::GetInstance().Initialize(settings);

		XREX::XREXContext::GetInstance().GetRenderingEngine().SetRenderingProcess(XREX::MakeSP<RenderingProcessContainer>(*this));
		XREX::XREXContext::GetInstance().SetLogicFunction(std::move(update));

	}

	~TutorialFrameworkXREXImplementation()
	{
	}


	void Start()
	{
		if (initialize != nullptr)
		{
			initialize();
		}

		XREX::XREXContext::GetInstance().Start();
	}

private:
	std::function<void()> initialize;
	std::function<void()> render;
};

/*
 *	Inherent from this class, override the virtual methods to do your work.
 */
class TutorialFramework
	: XREX::Noncopyable
{
public:
	TutorialFramework(XREX::Size<XREX::uint32, 2> screenSize)
		: framework(screenSize,
		[this] { Initialize(); },
		[this] (double current, double delta) { return Update(current, delta); },
		[this] { Render(); })
	{
	}
	/*
	 *	Start the program.
	 */
	void Start()
	{
		framework.Start();
	}

private:
	/*
	 *	Will be called when start.
	 */
	virtual void Initialize()
	{
	}
	/*
	 *	Will be called at the beginning of each frame.
	 *	@current: time from Start() was called.
	 *	@delta: time from last frame.
	 *	@return: false will exit the main loop.
	 */
	virtual bool Update(double current, double delta)
	{
		return true;
	}
	/*
	 *	Will be called after Update() in each frame.
	 */
	virtual void Render()
	{
	}

private:
	TutorialFrameworkXREXImplementation framework;
};