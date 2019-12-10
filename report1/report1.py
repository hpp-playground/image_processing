import cv2 as cv

FONT_SIZE = 4


def wirte_message(input_path, message, output_path):
    img = cv.imread(input_path)
    height, width = img.shape[:2]
    # define: putText(img, text, org, fontFace, fontScale, color,
    # thickness=None, lineType=None, bottomLeftOrigin=None)
    cv.putText(img, message, (width//4 - len(message)*FONT_SIZE, height//2),
               cv.FONT_HERSHEY_PLAIN, FONT_SIZE, (255, 255, 255),
               5, cv.LINE_AA)
    cv.imwrite(output_path, img)


if __name__ == "__main__":
    input_path = input("input file path: ")
    message = input("message:\n")
    output_path = input("output file path: ")
    wirte_message(input_path, message, output_path)
