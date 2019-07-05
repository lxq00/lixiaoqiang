#ifndef __ONVIFPROTOCOL_H__GetConfigurationOptions
#define __ONVIFPROTOCOL_H__GetConfigurationOptions
#include "CmdObject.h"


class CmdGetConfigurationOptions :public CmdObject
{
public:
	CmdGetConfigurationOptions(const std::string& _token) :token(_token)
	{
		action = "http://www.onvif.org/ver20/ptz/wsdl/GetConfigurationOptions";
	}
	virtual ~CmdGetConfigurationOptions() {}

	virtual std::string build(const URL& URL)
	{
		stringstream stream;

		stream << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
			<< "<s:Envelope " << onvif_xml_ns << ">"
			<< buildHeader(URL)
			<< "<s:Body>"
			<< "<GetConfigurationOptions xmlns=\"http://www.onvif.org/ver20/ptz/wsdl\">"
			<< "<ConfigurationToken>"<< token <<"</ConfigurationToken>"
			<< "</GetConfigurationOptions>"
			<< "</s:Body></s:Envelope>";

		return stream.str();
	}
	shared_ptr<OnvifClientDefs::ConfigurationOptions> options;
	virtual bool parse(const XMLObject::Child& body)
	{
		options = make_shared<OnvifClientDefs::ConfigurationOptions>();
		
		const XMLObject::Child& resp = body.getChild("tptz:GetConfigurationOptionsResponse");
		if(!resp)	return false;
		
		const XMLObject::Child& ptzopt = resp.getChild("tptz:PTZConfigurationOptions");
		if (!ptzopt)	return false;

		const XMLObject::Child& space = ptzopt.getChild("tt:Spaces");
		if (!space)	return false;


		const XMLObject::Child& abspan = space.getChild("tt:AbsolutePanTiltPositionSpace");
		if (abspan)
		{
			const XMLObject::Child& xrange = abspan.getChild("tt:XRange");
			if (xrange)
			{
				const XMLObject::Child& pmin = xrange.getChild("tt:Min");
				if (pmin)  options->absolute_pantilt_x.min = pmin.data().readFloat();

				const XMLObject::Child& pmax = xrange.getChild("tt:Max");
				if (pmax)  options->absolute_pantilt_x.max = pmax.data().readFloat();
			}

			const XMLObject::Child& yrange = abspan.getChild("tt:YRange");
			if (yrange)
			{
				const XMLObject::Child& pmin = yrange.getChild("tt:Min");
				if (pmin)  options->absolute_pantilt_y.min = pmin.data().readFloat();

				const XMLObject::Child& pmax = yrange.getChild("tt:Max");
				if (pmax)  options->absolute_pantilt_y.max = pmax.data().readFloat();
			}
		}

		const XMLObject::Child& abszoom = space.getChild("tt:AbsoluteZoomPositionSpace");
		if (abszoom)
		{
			const XMLObject::Child& xrange = abszoom.getChild("tt:XRange");
			if (xrange)
			{
				const XMLObject::Child& pmin = xrange.getChild("tt:Min");
				if (pmin)  options->absolute_zoom.min = pmin.data().readFloat();

				const XMLObject::Child& pmax = xrange.getChild("tt:Max");
				if (pmax)  options->absolute_zoom.max = pmax.data().readFloat();
			}
		}

		const XMLObject::Child& pantilt = space.getChild("tt:RelativePanTiltTranslationSpace");
		if (pantilt)
		{
			const XMLObject::Child& xrange = pantilt.getChild("tt:XRange");
			if (xrange)
			{
				const XMLObject::Child& pmin = xrange.getChild("tt:Min");
				if (pmin)  options->relative_pantilt_x.min = pmin.data().readFloat();

				const XMLObject::Child& pmax = xrange.getChild("tt:Max");
				if (pmax)  options->relative_pantilt_x.max = pmax.data().readFloat();
			}

			const XMLObject::Child& yrange = pantilt.getChild("tt:YRange");
			if (yrange)
			{
				const XMLObject::Child& pmin = yrange.getChild("tt:Min");
				if (pmin)  options->relative_pantilt_y.min = pmin.data().readFloat();

				const XMLObject::Child& pmax = yrange.getChild("tt:Max");
				if (pmax)  options->relative_pantilt_y.max = pmax.data().readFloat();
			}
		}

		const XMLObject::Child& panzoom = space.getChild("tt:RelativeZoomTranslationSpace");
		if (panzoom)
		{
			const XMLObject::Child& xrange = panzoom.getChild("tt:XRange");
			if (xrange)
			{
				const XMLObject::Child& pmin = xrange.getChild("tt:Min");
				if (pmin)  options->relative_zoom.min = pmin.data().readFloat();

				const XMLObject::Child& pmax = xrange.getChild("tt:Max");
				if (pmax)  options->relative_zoom.max = pmax.data().readFloat();
			}
		}

		const XMLObject::Child& continuous = space.getChild("tt:ContinuousPanTiltVelocitySpace");
		if (continuous)
		{
			const XMLObject::Child& xrange = continuous.getChild("tt:XRange");
			if (xrange)
			{
				const XMLObject::Child& pmin = xrange.getChild("tt:Min");
				if (pmin)  options->continuous_pantilt_x.min = pmin.data().readFloat();

				const XMLObject::Child& pmax = xrange.getChild("tt:Max");
				if (pmax)  options->continuous_pantilt_x.max = pmax.data().readFloat();
			}

			const XMLObject::Child& yrange = continuous.getChild("tt:YRange");
			if (yrange)
			{
				const XMLObject::Child& pmin = yrange.getChild("tt:Min");
				if (pmin)  options->continuous_pantilt_y.min = pmin.data().readFloat();

				const XMLObject::Child& pmax = yrange.getChild("tt:Max");
				if (pmax)  options->continuous_pantilt_y.max = pmax.data().readFloat();
			}
		}

		const XMLObject::Child& panzoomzoom = space.getChild("tt:ContinuousZoomVelocitySpace");
		if (panzoomzoom)
		{
			const XMLObject::Child& xrange = panzoomzoom.getChild("tt:XRange");
			if (xrange)
			{
				const XMLObject::Child& pmin = xrange.getChild("tt:Min");
				if (pmin)  options->continuous_zoom.min = pmin.data().readFloat();

				const XMLObject::Child& pmax = xrange.getChild("tt:Max");
				if (pmax)  options->continuous_zoom.max = pmax.data().readFloat();
			}
		}

		const XMLObject::Child& speed = space.getChild("tt:PanTiltSpeedSpace");
		if (speed)
		{
			const XMLObject::Child& xrange = speed.getChild("tt:XRange");
			if (xrange)
			{
				const XMLObject::Child& pmin = xrange.getChild("tt:Min");
				if (pmin)  options->pantilt_speed.min = pmin.data().readFloat();

				const XMLObject::Child& pmax = xrange.getChild("tt:Max");
				if (pmax)  options->pantilt_speed.max = pmax.data().readFloat();
			}
		}

		const XMLObject::Child& zoomspeed = space.getChild("tt:ZoomSpeedSpace");
		if (zoomspeed)
		{
			const XMLObject::Child& xrange = zoomspeed.getChild("tt:XRange");
			if (xrange)
			{
				const XMLObject::Child& pmin = xrange.getChild("tt:Min");
				if (pmin)  options->zoom_speed.min = pmin.data().readFloat();

				const XMLObject::Child& pmax = xrange.getChild("tt:Max");
				if (pmax)  options->zoom_speed.max = pmax.data().readFloat();
			}
		}

		const XMLObject::Child& timeout = ptzopt.getChild("tt:PTZTimeout");
		if (timeout)
		{
			const XMLObject::Child& pmin = timeout.getChild("tt:Min");
			if (pmin)  options->timeout.min = (float)pmin.data().readInt();

			const XMLObject::Child& pmax = timeout.getChild("tt:Max");
			if (pmax)  options->timeout.max = (float)pmax.data().readInt();
		}

		options->used = 1;

		return true;
	}

private:
	std::string token;
};



#endif //__ONVIFPROTOCOL_H__
