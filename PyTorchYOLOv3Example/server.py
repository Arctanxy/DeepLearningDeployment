from flask import Flask, render_template, request, redirect, url_for, make_response, jsonify
from werkzeug.utils import secure_filename
import os
import cv2
import time
from datetime import timedelta
from main import run, conf
ALLOWED_EXTENSIONS = set([
    "png","jpg","JPG","PNG", "bmp"
])

def is_allowed_file(filename):
    return '.' in filename and filename.rsplit('.', 1)[1] in ALLOWED_EXTENSIONS

app = Flask(__name__)

# 静态文件缓存过期时间
app.send_file_max_age_default = timedelta(seconds=1)

@app.route("/upload",methods = ['POST', 'GET'])
def upload():
    if request.method == "POST":
        f = request.files['file']
        if not ( f and is_allowed_file(f.filename)):
            return jsonify({
                "error":1001, "msg":"请检查上传的图片类型，仅限于png、PNG、jpg、JPG、bmp"
            })
        user_input = request.form.get("name")

        basepath = os.path.dirname(__file__)
        upload_path = os.path.join(basepath, "static/images",secure_filename(f.filename))
        f.save(upload_path)
        
        detected_path = os.path.join(basepath, "static/images", "output" + secure_filename(f.filename))
        run(upload_path, conf, detected_path)

        # return render_template("upload_ok.html", userinput = user_input, val1=time.time(), path = detected_path)
        path = "/images/" + "output" + secure_filename(f.filename)
        return render_template("upload_ok.html", path = path, val1 = time.time())
    return render_template("upload.html")


if __name__ == "__main__":
    app.run(host='0.0.0.0', port=8888, debug=True)
