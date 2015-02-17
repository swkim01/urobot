package org.urobot.controller;

import java.io.BufferedInputStream;
import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.DataInputStream;
import java.io.EOFException;
import java.io.IOException;
import java.io.InputStream;
import java.net.HttpURLConnection;
import java.net.URL;
import java.net.URLConnection;
import java.util.Enumeration;
import java.util.Hashtable;
import java.util.Properties;

import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
 
public class MultipartJpegInputStream extends DataInputStream {
	public static final String BOUNDARY_MARKER_PREFIX  = "--";
	public static final String BOUNDARY_MARKER_TERM    = BOUNDARY_MARKER_PREFIX;
	private static final String CONTENT_TYPE = "content-type";

	private boolean m_streamEnd = false;
	private boolean m_debug = false;
	private Hashtable mHeader;
	private String mBoundary = null;
	
	private final static int HEADER_MAX_LENGTH = 100;
	private final static int FRAME_MAX_LENGTH = 40000 + HEADER_MAX_LENGTH;
	private int mContentLength = -1;
	private static HttpURLConnection conn;
	
	public static MultipartJpegInputStream read(String url) {
		try {
			URL urladdr = new URL(url);
			conn = (HttpURLConnection) urladdr.openConnection();
			if (conn != null) {
				conn.setConnectTimeout(10000);
				if (conn.getResponseCode() == HttpURLConnection.HTTP_OK) {
					return new MultipartJpegInputStream(conn.getInputStream());
				}
			}
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		/*
		HttpResponse res;
		DefaultHttpClient httpclient = new DefaultHttpClient();
		try {
			res = httpclient.execute(new HttpGet(URI.create(url)));
			return new MultipartJpegInputStream(res.getEntity().getContent());
		} catch (ClientProtocolException e) {
		} catch (IOException e) {}
		*/
		return null;
	}
	
	public MultipartJpegInputStream(InputStream in) {
		super(new BufferedInputStream(in, FRAME_MAX_LENGTH));
	}
		
	public Hashtable readHeaders() throws IOException {
		Hashtable ht = new Hashtable();
		String response;
		boolean satisfied = false;
		
		do {
			//noinspection deprecation
			response = this.readLine();
			if (response == null) {
				m_streamEnd = true;
				break;
			} else if (response.equals("")) {
				if (satisfied) {
					break;
				} else {
					// Carry on...
				}
			} else {
				satisfied = true;
			}
			addPropValue(response, ht);
		} while (true);
		return ht;
	}
	
	protected static void addPropValue(String response, Hashtable ht) {
		int idx = response.indexOf(":");
		if (idx == -1) {
			return;
		}
		String tag = response.substring(0, idx);
		String val = response.substring(idx + 1).trim();
		ht.put(tag.toLowerCase(), val);
	}
	
	public static Hashtable readHeaders(URLConnection conn) {
		Hashtable ht = new Hashtable();
		int i = 0;
		do {
			String key = conn.getHeaderFieldKey(i);
			if (key == null) {
				if (i == 0) {
					i++;
					continue;
				} else {
					break;
				}
			}
			String val = conn.getHeaderField(i);
			ht.put(key.toLowerCase(), val);
			i++;
		} while (true);
		return ht;
	}
	
	public void skipToBoundary(String boundary) throws IOException {
		readToBoundary(boundary);
	}

	public byte[] readToBoundary(String boundary) throws IOException {
		ResizableByteArrayOutputStream baos = new ResizableByteArrayOutputStream();
		StringBuffer lastLine = new StringBuffer();
		int lineidx = 0;
		int chidx = 0;
		byte ch;
		do {
			try {
				ch = this.readByte();
			} catch (EOFException e) {
				m_streamEnd = true;
				break;
			}
			if (ch == '\n' || ch == '\r') {
				//
				// End of line... Note, this will now look for the boundary
                // within the line - more flexible as it can handle
                // arfle--boundary\n  as well as
                // arfle\n--boundary\n
				//
				String lls = lastLine.toString();
                int idx = lls.indexOf(BOUNDARY_MARKER_PREFIX);
                if (idx != -1) {
                    lls = lastLine.substring(idx);
                    if (lls.startsWith(boundary)) {
                        // Boundary found - check for termination
                        String btest = lls.substring(boundary.length());
                        if (btest.equals(BOUNDARY_MARKER_TERM)) {
                            m_streamEnd = true;
                        }
                        chidx = lineidx+idx;
                        break;
                    }
				}
				lastLine = new StringBuffer();
				lineidx = chidx + 1;
			} else {
				lastLine.append((char) ch);
			}
			chidx++;
			baos.write(ch);
		} while (true);
		//
		baos.close();
		baos.resize(chidx);
		return baos.toByteArray();
	}

	public boolean isAtStreamEnd() {
		return m_streamEnd;
	}
	
	public boolean checkHeaders(Hashtable ht) {
        if (m_debug) {
            for (Enumeration enm = ht.keys();enm.hasMoreElements();)
            {
                String hkey = (String)enm.nextElement();
                System.err.println("//   " + hkey + " = " + ht.get(hkey));
            }
        }
		// Work out the content type/boundary.
        try {
        	String connectionError = null;
        	String ctype = (String) ht.get(CONTENT_TYPE);
        	if (ctype == null) {
        		connectionError = "No main content type";
        	} else if (ctype.indexOf("text") != -1) {
        		String response;
        		//noinspection deprecation
        		while ((response = this.readLine()) != null) {
        			System.out.println(response);
        		}
        		connectionError = "Failed to connect to server (denied?)";
        	}
        	if (connectionError != null) {
        		System.err.println("// Check failed for " + connectionError);
        		return false;
        	}
        	// Boundary will always be something - '--' or '--foobar'
        	int bidx = ctype.indexOf("boundary=");
        	mBoundary = BOUNDARY_MARKER_PREFIX;
        	
        	if (bidx != -1) {
        		mBoundary = ctype.substring(bidx + 9);
        		ctype = ctype.substring(0, bidx);
        		// Remove quotes around boundary string [if present]
        		if (mBoundary.startsWith("\"") && mBoundary.endsWith("\"")) {
        			mBoundary = mBoundary.substring(1, mBoundary.length()-1);
        		}
        		if (!mBoundary.startsWith(BOUNDARY_MARKER_PREFIX)) {
        			mBoundary = BOUNDARY_MARKER_PREFIX + mBoundary;
        		}
        	} else {
        		System.err.println("// Check failed for boundary string");
        		return false;
        	}
        	// Now if we have a boundary, read up to that.
        	if (ctype.startsWith("multipart/x-mixed-replace")) {
        		mHeader = ht;
        		if (m_debug){
        			System.err.println("// Reading up to boundary");
        		}
        		skipToBoundary(mBoundary);
        	}
        } catch (IOException e) {
        	// TODO Auto-generated catch block
        	e.printStackTrace();
        }
		return true;
	}

    public Bitmap readMultipartJpegFrame() throws IOException {
    	String ctype = null;
    	int bidx;
    	
        mark(FRAME_MAX_LENGTH);
        if (mHeader == null) {
        	checkHeaders(readHeaders(conn));
        }
        if (mBoundary != null) {
        	Hashtable headers = readHeaders();
            if (m_debug) {
                System.err.println("// Chunk Headers received:");
                for (Enumeration enm = headers.keys();enm.hasMoreElements();) {
                    String hkey = (String)enm.nextElement();
                    System.err.println("//   " + hkey + " = " + headers.get(hkey));
                }
            }
			// Are we at the end of the m_stream?
			if (isAtStreamEnd()) {
				return null;
			}
			ctype = (String) headers.get(CONTENT_TYPE);
        }
		if (ctype == null) {
			System.err.println("No part content type");
			return null;
		}
		// Mixed Type -> just skip...
		if (ctype.startsWith("multipart/x-mixed-replace")) {
			// Skip
			bidx = ctype.indexOf("boundary=");
			mBoundary = ctype.substring(bidx + 9);
			if (m_debug) {
				System.err.println("// Skipping to boundary");
			}
			skipToBoundary(mBoundary);
			return null;
		} else {
			// Something we want to keep...
			if (m_debug) {
				System.err.println("// Reading to boundary");
			}
			byte[] frameData = readToBoundary(mBoundary);
			return BitmapFactory.decodeStream(new ByteArrayInputStream(frameData));
		}
    }
}

class ResizableByteArrayOutputStream extends ByteArrayOutputStream {
	
	public ResizableByteArrayOutputStream() {
		super();
	}

	public void resize(int size) {
		count = size;
	}
}
