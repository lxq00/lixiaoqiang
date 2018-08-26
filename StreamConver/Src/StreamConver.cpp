#include "StreamConver/StreamConver.h"
#include "ffmpegConver.h"

namespace Public {
namespace StreamConver {

struct Source::SourceInternal
{
	Source*				source;
	DataCallback		callback;
	EndoffileCallback	eofcallback;
	StartCallback		startcallback;
	ReadCallback		readcallback;

	std::string			filename;

	SourceInternal(Source* _source):source(_source){}
	~SourceInternal() { }
};

Source::Source(const StartCallback& startcallback)
{
	internal = new SourceInternal(this);
	internal->startcallback = startcallback;
}
void Source::input(void* data, uint32_t len)
{
	if (data != NULL && len >= 0)
	{
		internal->callback(this,data, len);
	}
}
void Source::setEndOfFile()
{
	internal->eofcallback(true);
}

Source::Source(const std::string& filename)
{
	internal = new SourceInternal(this);
	internal->filename = filename;
}
Source::Source(const ReadCallback& readcallback)
{
	internal = new SourceInternal(this);
	internal->readcallback = readcallback;
}
Source::~Source()
{
	SAFE_DELETE(internal);
}
void Source::setCallback(const DataCallback& dcallback,const EndoffileCallback& eofcallback)
{
	internal->callback = dcallback;
	internal->eofcallback = eofcallback;
}
bool Source::start()
{
	if (internal->startcallback != NULL)
	{
		return internal->startcallback(this);
	}

	return true;
}
bool Source::stop()
{
	return true;
}
std::string Source::filename() const
{
	return internal->filename;
}

struct Target::TargetInternal
{
	StreamFormat	format;
	OutputType		type;
	DataCallback	callback;
	std::string		filename;
};

Target::Target(const DataCallback& datacallback, StreamFormat streamformat)
{
	internal = new TargetInternal;
	internal->format = streamformat;
	internal->callback = datacallback;
	internal->type = OutputType_Buffer;
}
Target::Target(const std::string& filename, StreamFormat streamformat)
{
	internal = new TargetInternal;
	internal->filename = filename;
	internal->format = streamformat;
	internal->type = OutputType_File;
}
Target::~Target()
{
	SAFE_DELETE(internal);
}

bool Target::start()
{
	return true;
}
bool Target::stop()
{
	return true;
}
void Target::input(/*StreamType type, uint32_t timestmap, */void* data, uint32_t len)
{
	internal->callback(this,/*type, timestmap, */data, len);
}

Target::StreamFormat Target::format() const
{
	return internal->format;
}
Target::OutputType   Target::type() const
{
	return internal->type;
}
std::string Target::filename() const
{
	return internal->filename;
}
Target::DataCallback Target::callback() const
{
	return internal->callback;
}

class ConverInternal
{
public:
	shared_ptr<Source> source;
	shared_ptr<Target> target;
	shared_ptr<ffmpegConver> conver;

	bool start()
	{
		std::string sourcefile = source->filename();
		if (sourcefile == "")
		{
			if (source->internal->readcallback != NULL)
			{
				conver->setSourceReadcallback(source->internal->readcallback);
			}
			else
			{
				source->setCallback(Source::DataCallback(&ConverInternal::sourceDataCallback, this), Source::EndoffileCallback(&ConverInternal::endofCallback, this));
			}
		}
		else
		{
			conver->setSourceFile(sourcefile);
		}

		if (target->type() == Target::OutputType_File)
		{
			conver->setTargetFile(target->filename(),target->format());
		}
		else
		{
			conver->setTargetCallback(target->callback(), target->format(), target.get());
		}

		source->start();
		target->start();
		conver->start();

		return true;
	}
	bool stop()
	{
		conver->stop();
		target->stop();
		source->stop();

		return true;
	}

	void sourceDataCallback(Source*, void* data, uint32_t len)
	{
		conver->sourceInput(data, len);
	}
	void endofCallback(bool flag)
	{
		conver->setEndoffileFlag(flag);
	}
};

Conver::Conver(const shared_ptr<Source>& source, const shared_ptr<Target>& target)
{
	internal = new ConverInternal();
	internal->source = source;
	internal->target = target;
	internal->conver = make_shared<ffmpegConver>();
}
Conver::~Conver()
{
	internal->conver = NULL;
	internal->target = NULL;
	internal->conver = NULL;

	SAFE_DELETE(internal);
}

bool Conver::start(const EndOfFileCallback& eofcallback)
{
	if (internal->conver == NULL || internal->target == NULL || internal->source == NULL)
	{
		return false;
	}
	internal->conver->setEndoffileCallback(eofcallback, this);

	return internal->start();
}
bool Conver::stop()
{
	if (internal->conver == NULL || internal->target == NULL || internal->source == NULL)
	{
		return false;
	}

	return internal->stop();
}


}
}