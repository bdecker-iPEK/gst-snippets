/*
 * Simple GStreamer application template
 *
 * Copyright (c) 2014 Sebastian Dröge <sebastian@centricular.com>
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include <gst/gst.h>
#include <unistd.h>

static GstElement* m_pipeline;

static void on_pad_added(GstElement* rtpbin, GstPad* pad, GstElement* depay)
{
  if (!g_str_has_prefix(GST_OBJECT_NAME(pad), "recv_rtp_src_"))
    return;

  GstPad* sinkpad = gst_element_get_static_pad(depay, "sink");
  GstPad* peer = gst_pad_get_peer(sinkpad);
  if (peer) {
    gst_pad_unlink(peer, sinkpad);
    gst_object_unref(peer);
  }
  gst_pad_link(pad, sinkpad);
  gst_object_unref(sinkpad);
}

int
main (int argc, char **argv)
{
  gst_init(NULL, NULL);

  m_pipeline = gst_parse_launch("udpsrc address=224.0.0.1 name=udpsrc port=5600 multicast-iface=eth0 caps=\"application/x-rtp,media=(string)video,clock-rate=(int)90000,encoding-name=(string)H264,payload=(int)96 \"       rtph264depay name=depay ! h264parse ! omxh264dec ! queue ! fakesink       rtpbin name=rtpbin", NULL);
  g_assert(m_pipeline != NULL);

  GstElement* rtpbin = gst_bin_get_by_name(GST_BIN(m_pipeline), "rtpbin");
  GstElement* depay = gst_bin_get_by_name(GST_BIN(m_pipeline), "depay");
  GstElement* udpsrc = gst_bin_get_by_name(GST_BIN(m_pipeline), "udpsrc");
  gst_element_link_pads(udpsrc, "src", rtpbin, "recv_rtp_sink_%u");
  g_signal_connect (rtpbin, "pad-added", G_CALLBACK(on_pad_added), depay);
  gst_object_unref(rtpbin);
  gst_object_unref(depay);
  gst_object_unref(udpsrc);

  gst_element_set_state(m_pipeline, GST_STATE_PLAYING);
  sleep(5);
  //working as expected


  gst_element_set_state(m_pipeline, GST_STATE_READY);
  sleep(5);
  //working as expected


  gst_element_set_state(m_pipeline, GST_STATE_PLAYING);
  sleep(5);
  /*
  not working

0:00:10.189213850  2373  0x1124090 WARN         rtpjitterbuffer rtpjitterbuffer.c:587:calculate_skew: delta - skew: 0:00:05.018867690 too big, reset skew

(app:2373): GStreamer-CRITICAL **: 13:18:41.078: Padname recv_rtp_src_0_558365334_96 is not unique in element rtpbin, not adding
0:00:10.389308329  2373  0x1124090 WARN                 basesrc gstbasesrc.c:3072:gst_base_src_loop:<udpsrc> error: Internal data stream error.
0:00:10.389427131  2373  0x1124090 WARN                 basesrc gstbasesrc.c:3072:gst_base_src_loop:<udpsrc> error: streaming stopped, reason not-linked (-1)

(app:2373): GStreamer-CRITICAL **: 13:18:45.883: gst_pad_set_active: assertion 'GST_IS_PAD (pad)' failed

(app:2373): GStreamer-CRITICAL **: 13:18:45.884: gst_element_remove_pad: assertion 'GST_IS_PAD (pad)' failed

  */


  gst_element_set_state(m_pipeline, GST_STATE_NULL);
  gst_object_unref(m_pipeline);
  gst_deinit();
  return 0;
}
