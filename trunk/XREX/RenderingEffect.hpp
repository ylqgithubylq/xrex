#pragma once

#include "Declare.hpp"

#include <string>
#include <vector>




class EffectParameter
	: Noncopyable
{

public:
	EffectParameter(std::string name);
	virtual ~EffectParameter();

	virtual std::string const & GetName() const;

	virtual ElementType GetType() const;

	virtual void SetValue(bool const & value);
	virtual void SetValue(int32 const & value);
	virtual void SetValue(float const & value);
	virtual void SetValue(floatV2 const & value);
	virtual void SetValue(floatV3 const & value);
	virtual void SetValue(floatV4 const & value);
	virtual void SetValue(floatM44 const & value);
	//virtual void SetValue(TextureSP const & value);
// 	virtual void SetValue(std::vector<bool> const & value);
// 	virtual void SetValue(std::vector<int32> const & value);
// 	virtual void SetValue(std::vector<float> const & value);
// 	virtual void SetValue(std::vector<floatV2> const & value);
// 	virtual void SetValue(std::vector<floatV3> const & value);
// 	virtual void SetValue(std::vector<floatV4> const & value);
// 	virtual void SetValue(std::vector<floatM44> const & value);

	// a little brain fucking...
	// a helper struct to help return value automatically convert to sorts of types.
	struct ParameterValueAutoConverter
		: Noncopyable
	{
		virtual operator bool const &() const;
		virtual operator int32 const &() const;
		virtual operator float const &() const;
		virtual operator floatV2 const &() const;
		virtual operator floatV3 const &() const;
		virtual operator floatV4 const &() const;
		virtual operator floatM44 const &() const;
// 		virtual operator std::vector<bool> const &() const;
// 		virtual operator std::vector<int32> const &() const;
// 		virtual operator std::vector<float> const &() const;
// 		virtual operator std::vector<floatV2> const &() const;
// 		virtual operator std::vector<floatV3> const &() const;
// 		virtual operator std::vector<floatV4> const &() const;
	};
	/*
	 *	@return ParameterValueAutoConverter: a helper struct that can convert to other type automatically.
	 */
	virtual ParameterValueAutoConverter const & GetValue() const;

protected:
		std::string name_;
};

template <typename T>
class ConcreteEffectParameter
	: public EffectParameter
{
public:
	typedef T ValueType;

	explicit ConcreteEffectParameter(std::string const & name)
		: EffectParameter(name), converter_(this)
	{
	}
	virtual ElementType GetType() const override
	{
		return TypeToElementType<T>::Type;
	}

	virtual ParameterValueAutoConverter const & GetValue() const override
	{
		return converter_;
	}
	virtual void SetValue(T const & value) override
	{
		value_ = value;
	}


private:
	friend class ConcreteParameterValueAutoConverter;
	struct ConcreteParameterValueAutoConverter
		: public ParameterValueAutoConverter
	{
	public:
		explicit ConcreteParameterValueAutoConverter(ConcreteEffectParameter* parameter)
			: parameter_(parameter)
		{
		}
		virtual operator T const &() const override
		{
			return parameter_->value_;
		}
		ConcreteParameterValueAutoConverter(EffectParameter* parameter)
			: parameter_(parameter)
		{
		}
	private:
		ConcreteEffectParameter* parameter_;
	};


protected:
	T value_;
	ConcreteParameterValueAutoConverter converter_;
};





class RenderingEffect
	: Noncopyable
{


public:
	explicit RenderingEffect(std::string const & name);
	~RenderingEffect();

	void Initialize(std::vector<RenderingPassSP> const & passes);

	std::vector<EffectParameterSP> const & GetAllParameters()
	{
		return parameters_;
	}
	void AddParameter(EffectParameterSP const & parameter)
	{
		parameters_.push_back(parameter);
	}
	/*
	 *	@return null pointer if not exist.
	 */
	EffectParameterSP const & GetParameterByName(std::string const & name) const;

	uint32 GetPassCount() const
	{
		return passes_.size();
	}
	RenderingPassSP const & GetPass(uint32 pass) const
	{
		return passes_[pass];
	}

private:
	std::string name_;

	std::vector<EffectParameterSP> parameters_;
	std::vector<RenderingPassSP> passes_;
	bool initialized;
};





class RenderingPass
	: Noncopyable
{
public:
	explicit RenderingPass(RenderingEffect& effect);
	~RenderingPass();

	void Initialize(ProgramObjectSP& program);


	void Bind();

	ProgramObjectSP const & GetProgram() const
	{
		return program_;
	}

private:
	RenderingEffect& effect_;
	ProgramObjectSP program_;
	bool initialized_;
};
