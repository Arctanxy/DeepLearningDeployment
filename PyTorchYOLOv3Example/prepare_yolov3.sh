folder="PyTorch-YOLOv3"
file="yolov3.weights"
if [[ ! -d "$folder" ]]; then 
  echo $folder
  echo 'PyTorch-YOLOv3 not found '
  git clone https://github.com/eriklindernoren/PyTorch-YOLOv3.git 
fi
cp main.py PyTorch-YOLOv3/
cp server.py PyTorch-YOLOv3/
cp -r static PyTorch-YOLOv3/
cp -r templates PyTorch-YOLOv3
cd PyTorch-YOLOv3 
# pip install -r requirements.txt
cd weights 
if [ ! -f "$file" ]; then 
  echo 'yolov3.weights not found'
  ./download_weights.sh 
fi
