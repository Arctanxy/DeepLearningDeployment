from __future__ import division

from models import *
from utils.utils import *
from utils.datasets import *

import os
import sys
import time
import datetime
import argparse

from PIL import Image

import torch
from torchvision import datasets
from torch.autograd import Variable

import matplotlib.pyplot as plt
import matplotlib.patches as patches
from matplotlib.ticker import NullLocator

class custom_dict(dict):
    def __init__(self, d = None):
        if d is not None:
            for k,v in d.items():
                self[k] = v
        return super().__init__()

    def __key(self, key):
        return "" if key is None else key.lower()

    def __str__(self):
        import json
        return json.dumps(self)

    def __setattr__(self, key, value):
        self[self.__key(key)] = value

    def __getattr__(self, key):
        return self.get(self.__key(key))

    def __getitem__(self, key):
        return super().get(self.__key(key))

    def __setitem__(self, key, value):
        return super().__setitem__(self.__key(key), value)

conf = custom_dict({
    "model_def":"config/yolov3.cfg",
    "weights_path":"weights/yolov3.weights",
    "class_path":"data/coco.names",
    "conf_thres":0.8,
    "nms_thres":0.4,
    "img_size":416
})

def run(img_path, conf, target_path):
    device = torch.device("cuda" if torch.cuda.is_available() else "cpu")
    os.makedirs("output", exist_ok=True)
    classes = load_classes(conf.class_path)
    model = Darknet(conf.model_def, img_size=conf.img_size).to(device)

    if conf.weights_path.endswith(".weights"):
        # Load darknet weights
        model.load_darknet_weights(conf.weights_path)
    else:
        # Load checkpoint weights
        model.load_state_dict(torch.load(conf.weights_path))
    model.eval() 
    
    img = Image.open(img_path).convert("RGB")
    # print(img.size)
    img = img.resize(((img.size[0] // 32) * 32, (img.size[1] // 32) * 32))
    # print(img.size)
    # img_array = np.array(img)
    # print("target shape ",img_array.shape)
    # img = img.resize((conf.img_size, conf.img_size))
    img_array = np.array(img)
    img_tensor = pad_to_square(transforms.ToTensor()(img),0)[0].unsqueeze(0)
    conf.img_size = img_tensor.shape[2]
    
    with torch.no_grad():
        detections = model(img_tensor)
        detections = non_max_suppression(detections, conf.conf_thres, conf.nms_thres)[0]

    cmap = plt.get_cmap("tab20b")
    colors = [cmap(i) for i in np.linspace(0, 1, 20)]
    plt.figure()
    fig, ax = plt.subplots(1)
    ax.imshow(img_array)
    if detections is not None:
        # Rescale boxes to original image
        detections = rescale_boxes(detections, conf.img_size, img_array.shape[:2])
        unique_labels = detections[:, -1].cpu().unique()
        n_cls_preds = len(unique_labels)
        bbox_colors = random.sample(colors, n_cls_preds)
        for x1, y1, x2, y2, conf, cls_conf, cls_pred in detections:

            print("\t+ Label: %s, Conf: %.5f" % (classes[int(cls_pred)], cls_conf.item()))

            box_w = x2 - x1
            box_h = y2 - y1

            color = bbox_colors[int(np.where(unique_labels == int(cls_pred))[0])]
            # Create a Rectangle patch
            bbox = patches.Rectangle((x1, y1), box_w, box_h, linewidth=2, edgecolor=color, facecolor="none")
            # Add the bbox to the plot
            ax.add_patch(bbox)
            # Add label
            plt.text(
                x1,
                y1,
                s=classes[int(cls_pred)],
                color="white",
                verticalalignment="top",
                bbox={"color": color, "pad": 0},
            )

    # Save generated image with detections
    plt.axis("off")
    plt.gca().xaxis.set_major_locator(NullLocator())
    plt.gca().yaxis.set_major_locator(NullLocator())
    filename = img_path.split("/")[-1].split(".")[0]
    # plt.savefig(f"output/{filename}.png", bbox_inches="tight", pad_inches=0.0)
    plt.savefig(target_path, bbox_inches='tight', pad_inches=0.0)
    plt.close()



if __name__ == "__main__":
    run("data/samples/dog.jpg",conf)

